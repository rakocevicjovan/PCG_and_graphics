cbuffer LightBuffer : register(b0)
{
    float4 lightPos;
    float4 lightRGBI;

    float4 eyePos;  //4th element is elapsed, pack them up a bit

    float4 eccentricity;

    float4x4 lightView;
};

#define elapsed eyePos.w

Texture2D<float3> coverage : register(t0);
Texture2D<float3> carver : register(t1);
SamplerState CloudSampler : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : WPOS;
    float4 msPos : MSPOS;
};


///Constants

#define PI 3.14159f
#define CLOUD_BOTTOM eccentricity.y
#define CLOUD_TOP eccentricity.z
static const float sigExtinction = float3(0.5, 1, 2);

static const float3 UP = float3(0, 1., 0);
static const float earthRadius = 6000000;

static const float GAIN = 0.707f;
static const float LACUNARITY = 2.5789f;

static const float NUM_STEPS = 64.0;
//static const float CLOUD_THICKNESS = 64.f;
//static const float STEP_SIZE = CLOUD_THICKNESS / NUM_STEPS;

///Constants done //r(d) = e ^ ( -O`(t) * d);



float IntersectRaySphere(float3 ro, float3 rd, float3 center, float radius)
{
    float3 oc = ro - center;
    float a = dot(rd, rd);
    float b = 2.0 * dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return -1.0;
    }
    else
    {
        return (-b + sqrt(discriminant)) / (2.0 * a);
    }
}


float2 hollowSphereIntersectionDistances(float3 ro, float3 rd, float2 heightGradient)
{
    float3 earthCenter = float3(0, -earthRadius, 0);
    float r1 = heightGradient.x + earthRadius;
    float r2 = heightGradient.y + earthRadius;
}


bool IntersectRayPlane(float3 ro, float3 rd, float height, inout float tMax)
{
    float denom = dot(UP, rd);
    if (abs(denom) > 0.00001f) // your favorite epsilon
    {
        tMax = dot(UP, float3(0, height, 0) - ro) / denom; //float3 being a point on the cloud plane
        return tMax > 0.00001f;
    }
    return false;
}



float phaseMieHG(float cosTheta, float g)	// g[-1, 1], also called eccentricity
{
    float g2 = g * g;

    float numerator = 1.f - g2;

    float toPow = 1.f + g2 - 2.f * g * cosTheta;    //seen this use abs... not buying that it's required though
    float denominator = 4.f * PI * toPow * sqrt(toPow); //toPow * sqrt(toPow) or pow(toPow, 1.5) but I think this is faster...
			
    return numerator / denominator;
}



float visibility(float3 sampledPos, float3 lightPos)
{
    return smoothstep(-512.f, 512.f, sampledPos.x);
    //return 1.f;     //implement with 3d volume textures
}



float inScattering(float3 ro, float3 rd, float3 lightPos)    //is rd dir of light or ray...
{
    float3 lightToX = lightPos.xyz - ro; //could be the other way around... does it even matter?
    
    float distToLightSq = dot(lightToX, lightToX); //same as sqrt(dot(lightToX, lightToX)); 

    lightToX = lightToX / sqrt(distToLightSq);

    float cosTheta = dot(lightToX, rd);
    float phase = phaseMieHG(cosTheta, eccentricity.x);

    float v = visibility(ro, lightPos); //v = x.isInLight (sample shadow map - [0-1] to approximate soft shadow, otherwise ugly)
    
    float c_light_i = lightRGBI.xyz * lightRGBI.a / max(distToLightSq, 0.0001f); //lightRGBI.xyz / max(distToLightSq, 0.0001f);

	// * SUM[1, n] p(w, l_c_i) * v(x, p_light_i) * c_light_i(distance(x, p_light_i))
    return PI * (phase * v * c_light_i); //bracketed for each light in fact... but I use one only
}



float4 integrate(in float4 sum, in float dif, in float den, in float t)
{

}



float4 raymarch(float3 ro, float3 rd)
{
    float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float density = 0.f;
    float t = 0.f;
    float tMax = 0.f;

    bool intersects = IntersectRayPlane(ro, rd, eccentricity.z, tMax);

    if(!intersects)
        return sum;

    float adjStepSize = tMax / NUM_STEPS;

    for (int i = 0; i < NUM_STEPS; ++i)
    {
        if (density > .99)    break;  //this and maybe step(sampled r, .2) for cartoony, blocky mask with nice banding

        float3 curPos = ro + t * rd;

        float mask = coverage.Sample(CloudSampler, (curPos.xz - 512.f) / 1024.f).r;
        //multiply by double smoothstep (or other remap) to this to make a gradient
        //mask = min(smoothstep(eccentricity.y, eccentricity.z, curPos.y), mask);
        mask = min(smoothstep(CLOUD_TOP, CLOUD_BOTTOM, curPos.y), mask);

        density += mask * adjStepSize; //density += mask * phase;   //density += adjStepSize * mask * phase;
      
        sum += density; 

        t += adjStepSize;
    }

    //for thin cloud layers this is all right, as the angle to the light won't change significantly
    //float3 xToLight = normalize(lightPos.xyz - ro);
    //float cosTheta = dot(xToLight, rd);
    //float phase = phaseMieHG(cosTheta, eccentricity.x);

    float L_inscat = inScattering(ro, rd, lightPos.xyz);

    sum *= L_inscat;
    //sum *= phase;

    return sum;
}



float4 main(PixelInputType input) : SV_TARGET
{
    float3 rayDir = normalize(input.worldPos.xyz - eyePos.xyz);

    //colour = coverage.Sample(CloudSampler, (input.worldPos.xz - 512.f) / 1024.f);

    //return (float4) coverage.Sample(CloudSampler, (input.worldPos.xz - 512.f) / 1024.f).r;
    return raymarch(input.worldPos.xyz, rayDir);
}