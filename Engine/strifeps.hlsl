cbuffer LightBuffer : register(b0)
{
    float4 lightPos;
    float4 lightRGBI;
    float4 extinction;
    float4 eyePos;
    float4 eccentricity;
    float4 repeat;
    float4 opt;
    float4 misc;
    float4 ALTop;
    float4 ALBot;

    float4x4 camMatrix;
};

Texture2D<float3> weather       : register(t0);
Texture2D<float4> blueNoise     : register(t1);
Texture3D<float> baseVolTex     : register(t2);
Texture3D<float> fineVolTex     : register(t3);

//Texture3D<float4> baseVolTex    : register(t2);
//Texture3D<float4> fineVolTex    : register(t3);

SamplerState CloudSampler : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;

    //float3 normal : NORMAL;
    //float4 worldPos : WPOS;
    //float4 msPos : MSPOS;
};


/* Constants */
#define PI 3.14159f
#define NUM_STEPS 64.f
#define INV_NUM_STEPS (1.f / NUM_STEPS)
#define SHADOW_STEPS 4.f
#define EPS 0.00001f
#define HEIGHT_ZERO_INFLUENCE .333333f
#define MIN_CRUDE_STEPSIZE 5.f

#define MIP_HI_RES 0.f
#define MIP_LO_RES 2.f


/* Packing hax */
#define MIE_ECCENTRICITY eccentricity.x
#define CLOUD_BOTTOM eccentricity.y
#define CLOUD_TOP eccentricity.z
#define FOCAL_DEPTH eccentricity.w

#define GLOBAL_COVERAGE extinction.w
#define ELAPSED eyePos.w

#define BASE_REPEAT repeat.x
#define FINE_REPEAT repeat.y
#define CURL_REPEAT repeat.z
#define DENSITY_FAC repeat.w

#define SKY_COLOUR misc.xyz //(float3(135., 206., 250.) * .8f/ 255.)    // float3(misc.x, misc.y, misc.z)
#define MAX_VIS misc.w

//ambient light from Reinder's shader "Himalayas"   AMB_TOP (float3(149., 167., 200.) * (1.2 / 255.))   AMB_BOT (float3(39., 67., 87.) * (1.2 / 255.))
#define AMB_TOP ALTop.xyz
#define AMB_BOT ALBot.xyz
#define CARVING_THRESHOLD ALTop.w
#define TEXTURE_SPAN ALBot.w

//optimization
#define INV_LAYER_THICKNESS opt.x
#define INV_BASE_REPEAT opt.y
#define INV_FINE_REPEAT opt.z
#define PLANET_RADIUS opt.w

//static const float PLANET_RADIUS = 6400000.f; //6000000.0f is roughly equal to Earth's   1000000.f
static const float3 PLANET_CENTER = float3(0.f, -PLANET_RADIUS, 0.f);


//correct, just a typical range remap except it has clamp
float remap(float value, float min1, float max1, float min2, float max2)
{
    float perc = (value - min1) / (max1 - min1);
    return clamp(min2 + perc * (max2 - min2), min2, max2);
}

float remapNoClamp(float value, float min1, float max1, float min2, float max2)
{
    float perc = (value - min1) / (max1 - min1);
    return min2 + perc * (max2 - min2);
}


float remapFast01(float value, float min1, float max1)
{
    return clamp((value - min1) / (max1 - min1), 0., 1.);
}


//art of code, assumes normalized rd! slightly optimized by yours truly (avoids one sqrt and one mul), correct
float2 raySphereInt(float3 ro, float3 rd, float3 sc, float r)
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

    //float numerator = 1.f - g2;
    float toPow = 1.f + g2 - 2.f * g * cosTheta;
    //float denominator = 4.f * PI * toPow * sqrt(toPow); //toPow * sqrt(toPow) or pow(toPow, 1.5) but I think this is faster...

    return (1.f - g2) / (4.f * PI * toPow * sqrt(toPow));
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



float getCloudHeight(float3 p)
{
    return clamp((length(p - float3(0.0, -PLANET_RADIUS, 0.0)) - PLANET_RADIUS - CLOUD_BOTTOM) * INV_LAYER_THICKNESS /*/ (CLOUD_TOP - CLOUD_BOTTOM)*/, 0.0, 1.0);
}


//@todo great circle distance for x and z if required
float3 toSamplingCoordinates(float3 p)
{
    float3 uvw = float3(0.f, 0.f, 0.f);

    //uvw of a 3d texture goes as follows - u and w are x and z of a slice, and y is downwards slice by slice
    uvw.x = (p.x) * INV_BASE_REPEAT;
    //uvw.y = remap(length(p - float3(0.0, -PLANET_RADIUS, 0.0)) - PLANET_RADIUS, CLOUD_BOTTOM, CLOUD_TOP, 1.f, .5f);
    
    //uvw.y = (distance(p, float3(0.0, -PLANET_RADIUS, 0.0)) - PLANET_RADIUS - CLOUD_BOTTOM) / (CT - CB) * c;
    //opt version
    uvw.y = (distance(p, float3(0.0, -PLANET_RADIUS, 0.0)) - PLANET_RADIUS - CLOUD_BOTTOM) * INV_LAYER_THICKNESS * TEXTURE_SPAN;
    
    uvw.z = (p.z) * INV_BASE_REPEAT;

    return uvw.xzy;
}



float getCarver(float3 p, float cloudHeight)
{
    //float carvingFactor = remap(abs(.333f - cloudHeight), .0, .667, 0., 1.); //[.0, .667] to [0., 1.]
    //carvingFactor *= carvingFactor;

    float3 csc = float3(p.x, p.y, p.z) * INV_FINE_REPEAT; //cloudHeight

    //float4 s = fineVolTex.SampleLevel(CloudSampler, csc, MIP_HI_RES);
    //return s.r * s.g * s.b;       //return s.r * .5f + s.g * .25f + s.b * .125f;    //always 0 in fourth sample...
    return fineVolTex.SampleLevel(CloudSampler, csc, MIP_HI_RES);
}


#define DEN1 remap(sampled.r, sampled.g * .3 + sampled.b * .15 + sampled.a * .075, 1., 0., 1.)
#define DEN2 max(sampled.r, sampled.g)
#define DEN3 remap(sampled.r, (1.0 - (sampled.g * 0.625 + sampled.b * 0.25 + sampled.a * 0.125)), 1.0, 0.0, 1.0);
#define DEN4 sampled.r * sampled.b * 1.5f
#define DEN5 remap(sampled.r * sampled.g, sampled.b * .5f, 1., 0., 1.)

float sampleDensity(float3 p)
{
    float3 bsc = toSamplingCoordinates(p);
    float cloudHeight = getCloudHeight(p);

    //float3 wts = weather.Sample(CloudSampler, float2(bsc.x, bsc.z));
    //if(wts.r < EPS || wts.g < EPS)  //optimize... we NEED every optimization we can get...
        //return 0.f;
    //mask *= wts.r;
    //float texFilter = smoothstep(0.0, wts.g * .33, cloudHeight) * smoothstep(wts.g, wts.g * .66, cloudHeight);
    //myLayer = texFilter;

    // precomputed the mask... saves a ton of time - worst case frame duration went from ~83 to ~40

    //float4 sampled = baseVolTex.SampleLevel(CloudSampler, bsc, MIP_HI_RES); //fast (GPU skips deciding sample level), looks good
    //float iMask = DEN5;
    float iMask = baseVolTex.SampleLevel(CloudSampler, bsc, MIP_HI_RES);

    if(iMask < EPS)
        return 0.f;

    //for density and coverage modifications
    float heightFactor = smoothstep(0.f, 1.f - HEIGHT_ZERO_INFLUENCE, abs(HEIGHT_ZERO_INFLUENCE - cloudHeight));

    float mask = iMask;

    /* Modulating coverage over height */

    //mask = smoothstep(clamp(GLOBAL_COVERAGE * (1.f + heightFactor * heightFactor), 0., 1.), 1., mask);
    //mask = remapFast01(mask, clamp(GLOBAL_COVERAGE * (1.f + heightFactor * heightFactor), 0., 1.), 1.);

    mask = smoothstep(GLOBAL_COVERAGE, 1.f, mask * (1.f - heightFactor * heightFactor));
    //mask = remapFast01(mask * (1.f - heightFactor), GLOBAL_COVERAGE, 1.f);

    /* Modulating coverage over height */

    mask *= smoothstep(0.f, .333f, cloudHeight);

    if (iMask < CARVING_THRESHOLD)
        mask = max(0., mask - getCarver(p, cloudHeight)); //remap(mask, getCarver(p, cloudHeight), 1., 0., 1.); getCarver(p, cloudHeight);

    mask *= DENSITY_FAC;    //allows the user to control "precipitation" so to speak... aka gives grayer clouds

    return mask;
}


float sampleDensityLowRes(float3 p)
{
    float3 bsc = toSamplingCoordinates(p);
    float cloudHeight = getCloudHeight(p);
    
    //same optimization as above
    //float4 sampled = baseVolTex.SampleLevel(CloudSampler, bsc, MIP_LO_RES);
    //float mask = DEN5;
    float mask = baseVolTex.SampleLevel(CloudSampler, bsc, MIP_LO_RES);

    mask = remapFast01(mask, smoothstep(0., 1.f - HEIGHT_ZERO_INFLUENCE, abs(HEIGHT_ZERO_INFLUENCE - cloudHeight)), 1.);
    //mask = smoothstep(GLOBAL_COVERAGE, 1.f, mask * smoothstep(0., 1.f - HEIGHT_ZERO_INFLUENCE, abs(HEIGHT_ZERO_INFLUENCE - cloudHeight)));
    mask *= smoothstep(0., .3, cloudHeight);
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

    float t = 0.f;
    float density = 0.f;
    float3 transmittance = (float3) 1.f;
    float3 curPos = sampledPos;

    for (int i = 0; i < SHADOW_STEPS; ++i)
    {
        density = sampleDensityLowRes(curPos);
        transmittance *= BeerLambert(extinction.xyz, density * shadowstep);
        curPos += i * toLight * shadowstep;
        shadowstep *= 1.3f;
    }

    return transmittance; //assume no shadows from map for now, just self shadowing
}



float3 scattering(float3 x, float3 rd, float shadowstep, float cosTheta, float3 xToLight) //direction v is MINUS RD AND NOT RD!!! at least in formal notation in the book
{   
    // PI * SUM[1, n] [ p(w, l_c_i) * v(x, p_light_i) * c_light_i(distance(x, p_light_i)) ]
    
    //switched this out to assuming same light angle from the bottom to the top of the cloud layer...
    //float3 xToLight = normalize(lightPos.xyz - x);
    //float cosTheta = dot(rd, xToLight);
    
    //parts of the scattering formula per each light - we assume one light only

    // p(w, l_c_i) - Mie phase function, in the cloud accounting for complexity of real p.f. can be done using max or lerp
    
    float p = max(phaseMieHG(cosTheta, MIE_ECCENTRICITY), phaseMieHG(cosTheta, -0.33f * MIE_ECCENTRICITY));
    float3 v = visibility(x, xToLight, shadowstep);     //v(x, p_light_i)
    float3 incidentLight = lightRGBI.rgb * lightRGBI.a; //c_light_i(distance(x, p_light_i)) - known value range [32 000, 100 000] for the sun to earth lux

    //inscattering formula
    return PI * incidentLight * p * v;
}



float4 raymarch(float3 ro, float3 rd, float tMax)
{
    float4 sum = float4(0.0f, 0.0f, 0.0f, 1.0f);

    float adjStepSize = tMax * INV_NUM_STEPS;

    float t = adjStepSize * 0.5f; //to sample at middles of steps so backtracking doesn't go out of bounds

    int counter = 0;

    //only allocate once, assign as many times as needed...
    float density = 0.f;
    float crudeStepSize = 0.f;
    float curStepSize = 0.f;
    float3 curPos;
    float3 S;
    float3 cur_extinction;
    float3 cur_transmittance;
    float3 integratedScat;

    //assume it's the same everywhere because the difference is negligible, reuse
    float3 xToLight = normalize(lightPos.xyz - ro);
    float cosTheta = dot(rd, xToLight);

    bool crude = true;  //start with big steps

    [loop]
    for (int i = 0; i < NUM_STEPS * 3.f; ++i)
    {
        if (sum.a <= EPS || t > tMax)    //stay within cloud layer and only march until saturated
            break;

        curPos = ro + t * rd;   //position resulting from previous cycle

        if (crude)
        {
            density = sampleDensity(curPos);
            crudeStepSize = max(adjStepSize, MIN_CRUDE_STEPSIZE);
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
            density = sampleDensity(curPos);

            curStepSize = crudeStepSize * .2f;
            t += curStepSize;

            if(density > EPS)   //only integrate if there is enough density
            {
                //inscatter
                /*multiscattering  approximation... kinda sorta, probably won't do this
                for (int n = 0; n < 3; ++n)
                    S += scattering(curPos, rd, adjStepSize) * density * (extinction.xyz * pow(.8f, n));
                */

                //float3 ambientLight = lerp(AMB_BOT, AMB_TOP, getCloudHeight(curPos)); avoid creating new buffers 
                S = (scattering(curPos, rd, adjStepSize, cosTheta, xToLight) + lerp(AMB_BOT, AMB_TOP, getCloudHeight(curPos))) * density * extinction.xyz;
                cur_extinction = density * extinction.xyz; //max((float3) (EPS), density * extinction.xyz);
                cur_transmittance = BeerLambert(cur_extinction, curStepSize);

                //integrate
                integratedScat = (S - S * cur_transmittance) / cur_extinction;
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
    float t1 = raySphereInt(eyePos.xyz, rayDir, PLANET_CENTER, PLANET_RADIUS + CLOUD_BOTTOM).y;
    float t2 = raySphereInt(eyePos.xyz, rayDir, PLANET_CENTER, PLANET_RADIUS + CLOUD_TOP).y;

    //combine rayleigh with atmosphere somehow? phaseRayleigh(cosTheta)
    //float cosThetaRayleigh = dot(rayDir, normalize(lightPos.xyz - rayOrigin));
    //float4 sunContribution = phaseRayleigh(cosThetaRayleigh) * lightRGBI;

    float3 col = atmosphere();  // * (1.f - sunContribution.w) + (sunContribution.xyz);

    if (t1 > MAX_VIS)
        return float4(col, 1.f);

    //works only for from underneath the layer, but faster than covering all options
    if (t1 > EPS && t2 > EPS)
    {
        //mode 1 - chaotic?
        rayOrigin += (blueNoise.Sample(CloudSampler, ndc) * 2.f - 1.f);

        //mode 2 - along ray
        //rayOrigin += rayDir * (blueNoise.Sample(CloudSampler, ndc).b); 

        //float distanceFactor = t1 / MAX_VIS * 8.f;

        float4 cloudColour = raymarch(rayOrigin + rayDir * t1, rayDir, t2 - t1);
        col = col * cloudColour.w + cloudColour.xyz;
        col.xyz = pow(abs(col.xyz), 1.0 / 2.2);
    }

    return float4(col, 1.f);
}



/* FOR FUTURE REFERENCE */
//float4 sampled = myVolTex.Sample(CloudSampler, bsc);                  //bit slower but works well
//float4 sampled = myVolTex.Load(float4(ree.x, ree.y / 128, ree.z, 0)); //fastest, but not good enough for this purpose :(

//not a good approach... not at all! would need plenty of steps! (FOR SHADOWS)
//float sheetRadius = lightPos.y < CLOUD_TOP ? CLOUD_BOTTOM : CLOUD_TOP;
//float distToCloudEdge = RaySphereInt(sampledPos, toLight, PLANET_CENTER, PLANET_RADIUS + sheetRadius).y;