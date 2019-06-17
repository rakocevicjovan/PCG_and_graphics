cbuffer LightBuffer : register(b0)
{
    float4 lightPos;
    float4 lightRGBI;
    float4 extinction;
    float4 eyePos;
    float4 eccentricity;
    float4 repeat;
    //float4 bufferUmpteenth;

    float4x4 camMatrix;
};

Texture2D<float3> coverage : register(t0);
Texture2D<float1> carver : register(t1);
Texture2D<float4> blueNoise : register(t2);
Texture3D<float4> tex3d : register(t3);
SamplerState CloudSampler : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : WPOS;
    float4 msPos : MSPOS;
    float2 tex : TEXCOORD0;
};


///Constants
#define PI 3.14159f
#define MAX_VIS 20000
#define NUM_STEPS 64.f
#define SHADOW_STEPS 4.f
#define EPS 0.00001f
#define TEX_TILE_SIZE 2048.f
#define SKY_COLOUR (float3(135., 206., 250.) / 255.)

//and some packing hax
#define CLOUD_BOTTOM eccentricity.y
#define CLOUD_TOP eccentricity.z
#define FOCAL_DEPTH eccentricity.w
#define GLOBAL_COVERAGE extinction.w
#define ELAPSED eyePos.w

#define BASE_REPEAT repeat.x
#define FINE_REPEAT repeat.y
#define CURL_REPEAT repeat.z
#define DENSITY_FAC repeat.w



static const float3 UP = float3(0., 1., 0.);
static const float PLANET_RADIUS = 1000000.f; //6000000.0f is roughly equal to earth's 1000000
static const float3 PLANET_CENTER = float3(eyePos.x, -PLANET_RADIUS, eyePos.z);

static const float GAIN = 0.707f;
static const float LACUNARITY = 2.5789f;



//correct, just a typical range remap
float remap(float value, float min1, float max1, float min2, float max2)
{
    float perc = (value - min1) / (max1 - min1);
    return min2 + perc * (max2 - min2);
}


//art of code, assumes normalized rd! slightly optimized by yours truly (avoids one sqrt and one mul), correct
float2 RaySphereInt(float3 ro, float3 rd, float3 sc, float r)
{
    float t = dot(sc - ro, rd);     //project SCtoRO vector onto ray direction, get t of closest point
    float3 p = ro + rd * t;         //get that closest point, easy...
    
    float3 rToC = sc - p;           //vector from ray to sphere center...
    float y = dot(rToC, rToC);      //squared distance between closest point on ray and center of sphere

    float x = r * r - y;            //Pythagorean theorem, get squared distance from middle point to shell
    if(x < 0.f)
        return float2(-1.f, -1.f);

    x = sqrt(x);
    return float2(t - x, t + x);
}


//correct, but might want to use schlick instead as it is slightly faster
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
    //sigS = (0.490, 1.017, 2.339)
	//calculated using sigS(lambda) proportionate to 1.f / (lambda^4);

    return (3. / (16. * PI)) * (1. + cosTheta * cosTheta);
}


//placeholder, use rayleigh scattering / simple distance fog
float3 atmosphere()
{
    return SKY_COLOUR;
}



//correct but boring and sub optimal for sure... too many samples have to be taken for just shape - pack textures together!
float SampleDensityAtPosition(float3 p)
{
    /*
    float2 samplingCoord = p.xz / TEX_TILE_SIZE;

    float mask = coverage.Sample(CloudSampler, samplingCoord - ELAPSED * .01f).r; // 
    float hi_freq_mask = coverage.Sample(CloudSampler, samplingCoord - ELAPSED * .013f).r * .33f;
    mask = remap(mask, hi_freq_mask, .5, 0.0, 1.0); //mask - hi_freq_mask;

    //culls the weaker density samples by remapping them to 0 if under GLOBAL_COVERAGE
    mask = remap(mask, GLOBAL_COVERAGE, 1., 0., 1.);

    //mask *= smoothstep(CLOUD_BOTTOM, CLOUD_TOP, p.y);
    //mask *= smoothstep(CLOUD_TOP, CLOUD_BOTTOM, p.y);
    */

    float3 baseSamplingCoord;
    baseSamplingCoord.x = (p.x - BASE_REPEAT * 0.5f) / BASE_REPEAT;
    baseSamplingCoord.y = remap(distance(p, PLANET_CENTER) - PLANET_RADIUS, CLOUD_BOTTOM, CLOUD_TOP, 0., 1.);
    baseSamplingCoord.z = (p.z - BASE_REPEAT * 0.5f) / BASE_REPEAT;

    //baseSamplingCoord -= ELAPSED * .1f;

    float4 sampled = tex3d.Sample(CloudSampler, baseSamplingCoord.xzy);

    //base shape
    float mask = remap(sampled.x, sampled.y, 1., 0., 1.); // * .5 + sampled.z * .25 + sampled.w * .125

    //erode with coverage - survival of the densest heh...
    mask = remap(mask, GLOBAL_COVERAGE, 1., 0., 1.);

    //mask that only allows 0.0-0.3 range with [0, .1] increasing, [.1, .2] densest and [.2, .3] decreasing
    float stratus = remap(baseSamplingCoord.y, 0.f, .2f, 0.f, 1.f) * remap(baseSamplingCoord.y, .4f, .6f, 1.f, 0.f);

    mask *= (DENSITY_FAC / NUM_STEPS); //* stratus;
    return saturate(mask);
}


//absorption (Beer Lambert law) - from production volume rendering, Fong et al.
float3 BeerLambert(float3 sigA, float depth)
{
    return exp(-sigA * depth);
}


//slow... volume shadow map could be used with a separate pass to determine density! would save a lot of work!
float3 visibility(float3 sampledPos, float3 toLight)
{
    //return shadowMap(x, plighti) * volShad(x, plighti); standard way to do it, I assume no occlusion for now...
    
    //not a good approach... not at all! would need plenty of steps!
    //float sheetRadius = lightPos.y < CLOUD_TOP ? CLOUD_BOTTOM : CLOUD_TOP;
    //float distToCloudEdge = RaySphereInt(sampledPos, toLight, PLANET_CENTER, PLANET_RADIUS + sheetRadius).y;

    //reasonable approximation, I'd hope...

    float t = 0;
    float density = 0.f;
    float3 transmittance = (float3) 1.f;
    float shadowstep = 10.f;
    float3 curPos = sampledPos;

    for (int i = 0; i < SHADOW_STEPS; ++i)
    {
        density = SampleDensityAtPosition(curPos);
        transmittance *= BeerLambert(extinction.xyz, density);
        curPos += i * toLight * shadowstep;
        shadowstep *= 1.3f;
    }

    return max(1.f * transmittance, EPS); //assume no shadows from map for now, just self shadowing
}



float scattering(float3 x, float3 rd) //direction v is MINUS RD AND NOT RD!!! at least in formal notation in the book
{   
    // PI * SUM[1, n] [ p(w, l_c_i) * v(x, p_light_i) * c_light_i(distance(x, p_light_i)) ]
    
    //set up
    float3 xToLight = lightPos.xyz - x;
    float distToLight = length(xToLight);
    xToLight = xToLight / distToLight;
    float cosTheta = dot(rd, xToLight);
    
    //parts of the scattering formula per each light - we assume one light only
    
    // p(w, l_c_i) - Mie phase function, in the cloud
    //simple, one phase function    float p = phaseMieHG(cosTheta, eccentricity.x);   
    
    //accounting for backward spikes ("the raddish") in real cloud phase functions can be done in two ways:
    //float p = lerp(phaseMieHG(cosTheta, eccentricity.x), phaseMieHG(cosTheta, -0.625f * eccentricity.x), .5f);
    float p = max(phaseMieHG(cosTheta, eccentricity.x), phaseMieHG(cosTheta, -0.625f * eccentricity.x));

    float v = visibility(x, xToLight); // v(x, p_light_i)
    float incidentLight = lightRGBI.a; //c_light_i(distance(x, p_light_i)) - known value range [32 000, 100 000] for the sun to earth lux

    //inscattering formula
    return PI * p * v * incidentLight;
}



float4 integrate(in float4 sum, in float dif, in float den, in float t)
{}



float4 raymarch(float3 ro, float3 rd, float tMax)
{
    float4 sum = float4(0.0f, 0.0f, 0.0f, 1.0f); //float t_transmittance = 1.f; is sum.a

    float adjStepSize = tMax / NUM_STEPS;

    float t = 0.f; //adjStepSize * 0.5f; to sample at middles of steps

    for (int i = 0; i < NUM_STEPS; ++i)
    {
        if (sum.a < EPS)    break;  //this and maybe step(sampled r, .2) for cartoony, blocky mask with nice banding

        float3 curPos = ro + t * rd;
        t += adjStepSize;

        float density = SampleDensityAtPosition(curPos); //density += mask * phase;   //density += adjStepSize * mask * phase;

        if (density < EPS)  //might do rayleigh here... have to do SOMETHING, looks horrible now, black if no clouds :(
            continue;

        float3 cur_transmittance = BeerLambert(extinction.xyz, adjStepSize * density); //t or adjStepSize??? or just density???
        float inscat = scattering(curPos, rd); //inScattering(curPos, rd, lightPos.xyz);

        float3 integratedScat = (inscat - sum.a * inscat) / extinction.xyz;
        sum.rgb += sum.a * integratedScat; //supposedly better because energy conserving
        sum.a *= cur_transmittance;
    }
    return sum;
}



float4 main(PixelInputType input):SV_TARGET
{
    //correct, trivial
    float3 rayOrigin = eyePos.xyz;

    //correct
    float2 ndc = float2((input.tex.x * 2.f - 1.f) * (16.f / 9.f), input.tex.y * 2.f - 1.f);

    //correct however a bit wide for now...
    //focal depth might have to increase as widely cast rays can cause stretching artifacts in corners - MATCH WITH SCENE!!!
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
    if (t1 > 0. && t2 > 0.)
    {
        rayOrigin += (blueNoise.Sample(CloudSampler, ndc) * 2.f - 1.f); //blue noise offset
        float4 cloudColour = raymarch(rayOrigin + rayDir * t1, rayDir, t2 - t1);
        //col = cloudColour.xyz;
        col = col * cloudColour.w + cloudColour.xyz;
        col.xyz = pow(abs(col.xyz), 1.0 / 2.2);
    }

    return float4(col, 1.f);
}

  //col.xyz = lerp(col.xyz, atmosphere().xyz, 1 - col.a);   col.a = 1.f;