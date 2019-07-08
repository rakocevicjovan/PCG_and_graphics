cbuffer LightBuffer : register(b0)
{
    float4 lightPos;
    float4 lightRGBI;
    float4 extinction;
    float4 eyePos;
    float4 eccentricity;
    float4 repeat;

    float4x4 camMatrix;
};

Texture2D<float3> weather       : register(t0);
Texture2D<float4> blueNoise     : register(t1);
Texture3D<float4> baseVolTex    : register(t2);
Texture3D<float4> fineVolTex    : register(t3);

SamplerState CloudSampler : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : WPOS;
    float4 msPos : MSPOS;
    float2 tex : TEXCOORD0;
};


/* Constants */
#define PI 3.14159f
#define MAX_VIS 20000
#define NUM_STEPS 64.f
#define SHADOW_STEPS 4.f
#define SMALL_STEP_MAX 1.f
#define EPS 0.00001f
#define TEX_TILE_SIZE 2048.f
#define SKY_COLOUR (float3(135., 206., 250.) / 255.)
//from Reinder's shader
#define AMB_TOP (float3(149., 167., 200.) * (1.5 / 255.))
#define AMB_BOT (float3(39., 67., 87.) * (1.5 / 255.))

#define MIP_LO_RES 0
#define MIP_HI_RES 2

/* Packing hax */
#define CLOUD_BOTTOM eccentricity.y
#define CLOUD_TOP eccentricity.z
#define FOCAL_DEPTH eccentricity.w
#define GLOBAL_COVERAGE extinction.w
#define ELAPSED eyePos.w

#define BASE_REPEAT repeat.x
#define FINE_REPEAT repeat.y
#define CURL_REPEAT repeat.z
#define DENSITY_FAC repeat.w


static const float PLANET_RADIUS = 3000000.f; //6000000.0f is roughly equal to Earth's   1000000.f
static const float3 PLANET_CENTER = float3(0.f, -PLANET_RADIUS, 0.f);


//correct, just a typical range remap except it has clamp
float remap(float value, float min1, float max1, float min2, float max2)
{
    float perc = (value - min1) / (max1 - min1);
    return clamp(min2 + perc * (max2 - min2), min2, max2);  //clamp makes sense to me, but seems it wasn't used in Nubis
    //return min2 + perc * (max2 - min2);
}


//art of code, assumes normalized rd! slightly optimized by yours truly (avoids one sqrt and one mul), correct
float2 RaySphereInt(float3 ro, float3 rd, float3 sc, float r)
{
    float t = dot(sc - ro, rd);     //project SCtoRO vector onto ray direction, get t of closest point
    float3 p = ro + rd * t;         //get that closest point, easy...
    
    float3 rToC = sc - p;           //vector from ray to sphere center...
    float y = dot(rToC, rToC);      //squared distance between closest point on ray and center of sphere

    float x = r * r - y;            //Pythagorean theorem, get squared distance from middle point to shell
    if(x < EPS)
        return float2(-1.f, -1.f);

    x = sqrt(x);
    return float2(t - x, t + x);
}


//correct, but might want to use schlick instead as it is slightly faster, however it's inaccurate for g > .8
float phaseMieHG(float cosTheta, float g)	// g[-1, 1], also called eccentricity
{
    float g2 = g * g;

    float numerator = 1.f - g2;

    float toPow = 1.f + g2 - 2.f * g * cosTheta;        //seen this use abs... not buying that it's required though
    float denominator = 4.f * PI * toPow * sqrt(toPow); //toPow * sqrt(toPow) or pow(toPow, 1.5) but I think this is faster...

    return numerator / denominator;
}


//check if correct, use for atmospherical scattering and colour
float phaseRayleigh(float cosTheta)
{
    //sigS = (0.490, 1.017, 2.339)  calculated using sigS(lambda) proportionate to 1.f / (lambda^4);

    return (3. / (16. * PI)) * (1. + cosTheta * cosTheta);
}


//@todo use rayleigh scattering / magic numbers fog instead
float3 atmosphere()
{
    return SKY_COLOUR;
}


//@todo great circle distance for x and z if required
float3 toSamplingCoordinates(float3 p)
{
    float3 uvw = float3(0.f, 0.f, 0.f);

    //uvw of a 3d texture goes as follows - u and w are x and z of a slice, and y is downwards slice by slice
    uvw.x = ((p.x - BASE_REPEAT * 0.5f) % BASE_REPEAT) / BASE_REPEAT;
    uvw.y = remap(length(p - float3(0.0, -PLANET_RADIUS, 0.0)) - PLANET_RADIUS, CLOUD_BOTTOM, CLOUD_TOP, 1.f, 0.f);
    uvw.z = ((p.z - BASE_REPEAT * 0.5f) % BASE_REPEAT) / BASE_REPEAT;

    return uvw.xzy;
}



float getCarver(float3 p, float cloudHeight)
{
    //float carvingFactor = remap(abs(.333f - cloudHeight), .0, .667, 0., 1.); //[.0, .667] to [0., 1.]
    //carvingFactor *= carvingFactor;

    float ree = FINE_REPEAT;   //BASE_REPEAT * .0625f;
    float3 csc = float3((p.x % ree) / ree, (p.y % ree) / ree, (p.z % ree) / ree); //cloudHeight

    float4 s = fineVolTex.SampleLevel(CloudSampler, csc, 0);

    return s.r * s.g * s.b;    //always 0 in fourth sample...
}


#define STM1 remap(sampled.r, sampled.g * .3 + sampled.b * .15 + sampled.a * .075, 1., 0., 1.)
#define STM2 max(sampled.r, sampled.g)

#define STM3 (sampled.g * 0.625 + sampled.b * 0.25 + sampled.a * 0.125)
#define STM33 remap(sampled.r, (1.0 - STM3), 1.0, 0.0, 1.0);

#define STM4 sampled.r * sampled.g


float getCloudHeight(float3 p)
{
    return clamp((length(p - float3(0.0, -PLANET_RADIUS, 0.0)) - PLANET_RADIUS - CLOUD_BOTTOM) / (CLOUD_TOP - CLOUD_BOTTOM), 0.0, 1.0);
}

float SampleDensity(float3 p, float curDen)
{
    //this works... however it's slower as it adds a sample each time, still ok at half resolution
    //float3 wts = weather.Sample(CloudSampler, (float2(p.x, p.z) % BASE_REPEAT) / BASE_REPEAT);
    //if(wts.r < EPS || wts.g < EPS)  //optimize... we NEED every optimization we can get...
        //return 0.f;
    //float myLayer = smoothstep(0.0, wts.g * .33, cloudHeight) * smoothstep(wts.g, wts.g * .66, cloudHeight);
    //mask *= wts.r;

    float3 bsc = toSamplingCoordinates(p);
    float cloudHeight = getCloudHeight(p);

    //height variability
    float heightZeroLayer = .333333f;
    float heightFactor = abs(heightZeroLayer - cloudHeight);
    heightFactor = smoothstep(0., 1.f - heightZeroLayer, heightFactor);

    float myLayer = smoothstep(0., .3, cloudHeight); //reduce density at the bottom, not all the way to 0 though
    myLayer *=  smoothstep(1.0, 0.7, cloudHeight);       //and slightly at the top

    float4 sampled = baseVolTex.SampleLevel(CloudSampler, bsc, 0);  //fast (GPU skips deciding sample level), looks good
    
    float iMask = STM4;
    float mask = iMask;

    float modifiedCoverage = clamp(GLOBAL_COVERAGE * (1.f + heightFactor * heightFactor), 0., 1.);

    //@TODO CHOOSE ONE - they are very, very similar with smoothstep being a bit fuzzier and softer, I think?
    mask = remap(mask, modifiedCoverage, 1., 0., 1.);
    //mask = smoothstep(modifiedCoverage, 1., mask);

    //this makes the "edges" fuzzy... meaning that eroding will hit them harder
    mask *= myLayer; //using these before global coverage kills the fuzz (less dense areas get carved away)

    //@TODO erode somehow...
    float cInt = smoothstep(.5, 0., iMask);

    if(cInt > 0.f && mask > EPS)
    {
        //mask = max(0., mask - getCarver(p, cloudHeight));
        mask = remap(mask, getCarver(p, cloudHeight), 1., 0., 1.);
    }

    mask *= DENSITY_FAC;    //allows the user to control "precipitation" so to speak... aka gives grayer clouds

    return mask;
}



float SampleDensityLowRes(float3 p)
{
    float3 bsc = toSamplingCoordinates(p);
    float4 sampled = baseVolTex.SampleLevel(CloudSampler, bsc, 2);

    float atmoHeight = length(p - float3(0.0, -PLANET_RADIUS, 0.0)) - PLANET_RADIUS;
    float cloudHeight = clamp((atmoHeight - CLOUD_BOTTOM) / (CLOUD_TOP - CLOUD_BOTTOM), 0.0, 1.0);

    float myLayer = smoothstep(0., .3, cloudHeight);

    float mask = STM4;
    mask = remap(mask, GLOBAL_COVERAGE, 1., 0., 1.);
    mask *= myLayer;
    mask *= DENSITY_FAC;
    return min(mask, 1.);
}


//absorption (Beer Lambert law) - from production volume rendering, Fong et al.
float3 BeerLambert(float3 sigA, float depth)
{
    return exp(-sigA * depth);
}


//slow... volume shadow map could be used with a separate pass to determine density! would save a lot of work!
float3 visibility(float3 sampledPos, float3 toLight, float shadowstep)
{
    //return shadowMap(x, plighti) * volShad(x, plighti); standard way to do it, I assume no occlusion for now...
    
    //not a good approach... not at all! would need plenty of steps!
    //float sheetRadius = lightPos.y < CLOUD_TOP ? CLOUD_BOTTOM : CLOUD_TOP;
    //float distToCloudEdge = RaySphereInt(sampledPos, toLight, PLANET_CENTER, PLANET_RADIUS + sheetRadius).y;

    //reasonable approximation, I'd hope...

    float t = 0.f;
    float density = 0.f;
    float3 transmittance = (float3) 1.f;
    float3 curPos = sampledPos;

    for (int i = 0; i < SHADOW_STEPS; ++i)
    {
        density = SampleDensityLowRes(curPos); //SampleDensity(curPos);
        transmittance *= BeerLambert(extinction.xyz, density * shadowstep);
        curPos += i * toLight * shadowstep;
        shadowstep *= 1.3f;
    }

    return transmittance; //assume no shadows from map for now, just self shadowing
}



float3 scattering(float3 x, float3 rd, float shadowstep) //direction v is MINUS RD AND NOT RD!!! at least in formal notation in the book
{   
    // PI * SUM[1, n] [ p(w, l_c_i) * v(x, p_light_i) * c_light_i(distance(x, p_light_i)) ]
    
    //set up @TODO might switch to a directional light it's a lot of operations for tiny gain in precision!
    float3 xToLight = normalize(lightPos.xyz - x);
    float cosTheta = dot(rd, xToLight);
    
    //parts of the scattering formula per each light - we assume one light only

    // p(w, l_c_i) - Mie phase function, in the cloud
    //accounting for backward spikes ("the raddish") in real cloud phase functions can be done in two ways:
    //float p = lerp(phaseMieHG(cosTheta, eccentricity.x), phaseMieHG(cosTheta, -0.625f * eccentricity.x), .5f);
    float p = max(phaseMieHG(cosTheta, eccentricity.x), phaseMieHG(cosTheta, -0.33f * eccentricity.x));

    float3 v = visibility(x, xToLight, shadowstep);     //v(x, p_light_i)

    float3 incidentLight = lightRGBI.rgb * lightRGBI.a; //c_light_i(distance(x, p_light_i)) - known value range [32 000, 100 000] for the sun to earth lux

    //inscattering formula
    return PI * incidentLight * p * v;
}



float4 raymarch(float3 ro, float3 rd, float tMax)
{
    float4 sum = float4(0.0f, 0.0f, 0.0f, 1.0f);

    float adjStepSize = tMax / NUM_STEPS;

    float t = adjStepSize * 0.5f; //to sample at middles of steps so backtracking doesn't go out of bounds

    float3 curPos;
    //float acc_density = 0.f;  //gives ok results too
    float density = 0.f;
    float crudeStepSize = 0.f;
    int counter = 0;

    bool crude = true;  //start with big steps

    [loop]
    for (int i = 0; i < NUM_STEPS * 3.f; ++i)
    {
        if (sum.a <= 0. || t > tMax)    //stay within cloud layer and only march until saturated
            break;

        curPos = ro + t * rd;   //position resulting from previous cycle

        if (crude)
        {
            density = SampleDensity(curPos, sum.a);    //low res maybe
            crudeStepSize = max(adjStepSize, 5.f);
            crude = (density < EPS);

            if (crude)              //keep stepping broadly when no density is found
                t += crudeStepSize;
            else                    //step finely for a while, going back first
            {
                t -= (crudeStepSize * 1.0001f);
                counter = 0; //fine step counter resets
            }
        }
        else
        {
            density = SampleDensity(curPos, sum.a);
            //acc_density += density;

            float curStepSize = crudeStepSize * .2f; //best just set min layer width tbh...
            t += curStepSize;

            if(density > 0.)   //only integrate if there is enough density
            {
                float3 ambientLight = lerp(AMB_BOT, AMB_TOP, getCloudHeight(curPos));
                //inscatter
                float3 S = float3(0., 0., 0.);

                /*multiscattering  approximation... kinda sorta, probably won't do this
                for (int n = 0; n < 3; ++n)
                    S += scattering(curPos, rd, adjStepSize) * density * (extinction.xyz * pow(.8f, n));
                */

                S = (scattering(curPos, rd, adjStepSize) + ambientLight) * density * extinction.xyz;
                float3 cur_extinction = density * extinction.xyz; //max((float3) (EPS), density * extinction.xyz);
                float3 cur_transmittance = BeerLambert(cur_extinction, curStepSize);
                
                //integrate
                float3 integratedScat = (S - S * cur_transmittance) / cur_extinction;
                sum.rgb += sum.a * integratedScat; //energy conserving
                sum.a *= cur_transmittance.g;
                counter = 0;
            }
            else
            {
                ++counter; 
            }
            
            if (counter >= 10)  //10 steps with insufficient density, back to the crude loop
                crude = true;
        }
    }
    return sum;
}



float4 main(PixelInputType input):SV_TARGET
{
    //correct, trivial
    float3 rayOrigin = eyePos.xyz;

    //correct
    float2 ndc = float2((input.tex.x * 2.f - 1.f) * (16.f / 9.f), input.tex.y * 2.f - 1.f);

    //correct, match focal depth to scene (this is for the user of the tool not me)
    float3 rayDir = normalize(camMatrix._11_12_13 * ndc.x + camMatrix._21_22_23 * ndc.y + camMatrix._31_32_33 * FOCAL_DEPTH);

    //bottom and top of a single cloud layer... might be faster to use onioning... but this works nicely!
    float t1 = RaySphereInt(rayOrigin, rayDir, PLANET_CENTER, PLANET_RADIUS + CLOUD_BOTTOM).y;
    float t2 = RaySphereInt(rayOrigin, rayDir, PLANET_CENTER, PLANET_RADIUS + CLOUD_TOP).y;

    //combine rayleigh with atmosphere somehow? phaseRayleigh(cosTheta)
    //float cosThetaRayleigh = dot(rayDir, normalize(lightPos.xyz - rayOrigin));
    //float4 sunContribution = phaseRayleigh(cosThetaRayleigh) * lightRGBI;
    float3 col = atmosphere();// * (1.f - sunContribution.w) + (sunContribution.xyz);

    if (t1 > MAX_VIS)
        return float4(col, 1.f);

    //works only for from underneath the layer, but faster than covering all options
    if (t1 > EPS && t2 > EPS)
    {
        //mode 1 - chaotic?
        //rayOrigin += (blueNoise.Sample(CloudSampler, ndc) * 2.f - 1.f);

        //mode 2 - along ray
        rayOrigin += rayDir * (blueNoise.Sample(CloudSampler, ndc).b); 

        float4 cloudColour = raymarch(rayOrigin + rayDir * t1, rayDir, t2 - t1);
        col = col * cloudColour.w + cloudColour.xyz;
        col.xyz = pow(abs(col.xyz), 1.0 / 2.2);
    }

    return float4(col, 1.f);
}







/*
// Noise generation functions (by iq)
float hash(float n)
{
    return frac(sin(n) * 43758.5453);
}

float hash(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453123);
}

float noise(in float3 x)
{
    float3 p = floor(x);
    float3 f = frac(x);
    f = f * f * (3.0 - 2.0 * f);
    
    //return fineVolTex.SampleLevel(CloudSampler, p + f + 0.5f / 32.f, 0.f).x;

    float2 uv = (p.xy + float2(37.0, 17.0) * p.z) + f.xy;
    float2 rg = fineVolTex.SampleLevel(CloudSampler, float3((uv + 0.5) / 256.0, 0.f), 0.0).yx;
    return lerp(rg.x, rg.y, f.z);
}

float fbm(float3 p)
{
    float3x3 m = float3x3(0.00, 0.80, 0.60,
              -0.80, 0.36, -0.48,
              -0.60, -0.48, 0.64);
    float f;
    f = 0.5000 * noise(p);
    p = mul(m, p) * 2.02;
    f += 0.2500 * noise(p);
    p = mul(m, p) * 2.03;
    f += 0.1250 * noise(p);
    return f;
}
*/

/*
float SampleDensityNubis(float3 p, float curDen)
{
    float3 bsc = toSamplingCoordinates(p);
    float atmoHeight = length(p - float3(0.0, -PLANET_RADIUS, 0.0)) - PLANET_RADIUS;
    float cloudHeight = clamp((atmoHeight - CLOUD_BOTTOM) / (CLOUD_TOP - CLOUD_BOTTOM), 0.0, 1.0);

    float4 sampled = baseVolTex.SampleLevel(CloudSampler, bsc, 0);  //rgba, r being perling-worley, rest worley at incr. freq.

    float stratocumulus = remap(cloudHeight, 0., .2, 0., 1.) * remap(cloudHeight, .6, .4, 0., 1.);
    
    if(stratocumulus < EPS)
        return 0.;

    float mask = sampled.g * sampled.b;
    mask = remap(mask * stratocumulus, GLOBAL_COVERAGE, 1., 0., 1.);

    mask *= smoothstep(0., 1., cloudHeight);

    //carve(p, cloudHeight, mask);

    mask *= DENSITY_FAC;

    return mask;
}
*/


/* FOR FUTURE REFERENCE */
//float4 sampled = myVolTex.Sample(CloudSampler, bsc);                  //bit slower but works well
//float4 sampled = myVolTex.Load(float4(ree.x, ree.y / 128, ree.z, 0)); //fastest, but not good enough for this purpose :(