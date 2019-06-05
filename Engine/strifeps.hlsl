cbuffer LightBuffer : register(b0)
{
    float4 lightPos;
    float4 lightRGBI;
    float4 extinction;

    float4 eyePos;  //4th element is elapsed, pack them up a bit

    float4 eccentricity;        //eccentricity, cloud bottom layer, cloud top layer, focal depth

    float4x4 lightView;
    float4x4 camMatrix;
};

Texture2D<float3> coverage : register(t0);
Texture2D<float1> carver : register(t1);
Texture2D<float4> blueNoise : register(t2);
SamplerState CloudSampler : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : WPOS;
    float4 msPos : MSPOS;
    float2 tex : TEXCOORD0;
};


///Constants and hax
#define PI 3.14159f
#define MAX_VIS 20000
#define SHADOW_STEPS 6.f
#define EPS 0.00001f

#define CLOUD_BOTTOM eccentricity.y
#define CLOUD_TOP eccentricity.z
#define FOCAL_DEPTH eccentricity.w

#define ELAPSED eyePos.w

static const float3 UP = float3(0., 1., 0.);
static const float PLANET_RADIUS = 1000000.f; //6000000.0f; 1000000
static const float3 PLANET_CENTER = float3(0., -PLANET_RADIUS, 0.);

static const float GAIN = 0.707f;
static const float LACUNARITY = 2.5789f;

static const float NUM_STEPS = 64.0;
//static const float CLOUD_THICKNESS = 64.f;
//static const float STEP_SIZE = CLOUD_THICKNESS / NUM_STEPS;



//just a typical remap function, correct
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



//correct
float phaseMieHG(float cosTheta, float g)	// g[-1, 1], also called eccentricity
{
    float g2 = g * g;

    float numerator = 1.f - g2;

    float toPow = 1.f + g2 - 2.f * g * cosTheta;        //seen this use abs... not buying that it's required though
    float denominator = 4.f * PI * toPow * sqrt(toPow); //toPow * sqrt(toPow) or pow(toPow, 1.5) but I think this is faster...

    return numerator / denominator;
}



float phaseRayleigh(float cosTheta)
{
    //sigS = (0.490, 1.017, 2.339)
	//calculated using sigS(lambda) proportionate to 1.f / (lambda^4);

    return (3. * (1. + cosTheta * cosTheta)) / (16. * PI);
}



float4 atmosphere(float t)
{

    return float4(0.1f, 0.1f, t / 40000.f, .5f);
}



//correct but boring and sub optimal for sure... too many samples have to be taken for just shape - pack textures together!
float SampleDensityAtPosition(float3 p)
{
    float2 samplingCoord = (p.xz - 512.f) / 1024.f;

    float mask = coverage.Sample(CloudSampler, samplingCoord).r;
    //float hi_freq_mask = carver.Sample(CloudSampler, samplingCoord).r;
    //mask = remap(mask, hi_freq_mask * p1.z), .1, 0.0, 1.0);

    //mask = min(smoothstep(CLOUD_BOTTOM, CLOUD_TOP, p.y), mask);
    //mask = min(smoothstep(CLOUD_TOP, CLOUD_BOTTOM, p.y), mask);

    return mask;
}


//eeeh?
float visibility(float3 sampledPos, float3 lightPos, float tMax)
{
    //return smoothstep(-512.f, 512.f, sampledPos.x);
    float density = 0.f;
    float3 toLight = normalize(lightPos - sampledPos);

    float t = 0;
    float shadowstep = tMax / SHADOW_STEPS;

    for (int i = 0; i < SHADOW_STEPS; ++i)
    {
        float3 curPos = sampledPos + i * toLight;
        density += SampleDensityAtPosition(curPos);
    }

    return max(0., 1.f - exp(-density)); //implement with 3d volume textures or marching? idk
}



//check check check!!! lighting is NOT correct
float inScattering(float3 x, float3 rd, float3 lightPos)
{
    // PI * SUM[1, n] [ p(w, l_c_i) * v(x, p_light_i) * c_light_i(distance(x, p_light_i)) ]

    // p(w, l_c_i) - Mie phase function, in the cloud
    float3 lightToX = lightPos.xyz - x; //vector to light
    float distToLightSq = dot(lightToX, lightToX); //squared distance to light...y tho?
    lightToX = lightToX / sqrt(distToLightSq);
    float cosTheta = dot(lightToX, rd);
    float phase = phaseMieHG(cosTheta, eccentricity.x);
        
    // v(x, p_light_i)
    float v = 1.f;
    //visibility(x, lightPos, CLOUD_TOP - x.y); this is shadowing on it's own and not very good...

    //sample shadow map - [0, 1] to approximate soft shadow, otherwise ugly OR sample towards light for density (slow?)
    
    //lightRGBI.xyz / max(distToLightSq, 0.0001f); however this is a 
    // c_light_i(distance(x, p_light_i)) - known value range [32 000, 100 000] for the sun to earth lux
    float c_light_i = lightRGBI.a; //lightRGBI.a; 1 //make color dependant lightRGBI.xyz * 

    return PI * (phase * v * c_light_i); //bracketed for each light in fact... but I use one only
}



//absorption (Beer Lambert law) - from production volume rendering, Fong et al.
float3 BeerLambert(float3 sigA, float depth)
{
    return exp(-sigA * depth);
}



void scattering(float3 x, float3 rd, ) //direction v is MINUS RD AND NOT RD!!! at least in formal notation in the book
{
    float result = PI;
    
    float3 xToLight = lightPos.xyz - x;
    float distToLight = length(xToLight);
    xToLight = xToLight / distToLight;

    float cosTheta = dot(rd, xToLight);
    float p = phaseMieHG(cosTheta, eccentricity.x);

    float incidentLight = lightRGBI.a;
    

    return result;
}



float4 integrate(in float4 sum, in float dif, in float den, in float t)
{}



float4 raymarch(float3 ro, float3 rd, float tMax)
{
        float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f); //or start with full light... idk

        float adjStepSize = tMax / NUM_STEPS;

        float t = adjStepSize * 0.5f; //sample at middles of steps

        for (int i = 0; i < NUM_STEPS; ++i)
        {
        //if (sum.a > 1.f - EPS)    break;  //this and maybe step(sampled r, .2) for cartoony, blocky mask with nice banding

            float3 curPos = ro + t * rd;

            float density = SampleDensityAtPosition(curPos); //density += mask * phase;   //density += adjStepSize * mask * phase;

            bool continueDoesntWorkMicrosoftPls = density > EPS;

            if (continueDoesntWorkMicrosoftPls)
            {
                float3 transmittance = BeerLambert(extinction.xyz, adjStepSize).xyz; //t or adjStepSize??? or just density???
                float inscat = inScattering(curPos, rd, lightPos.xyz);
                sum.rgb += transmittance * inscat; //density *
                sum.a += density;
            }

            t += adjStepSize;
        }

        sum.rgb = pow(abs(sum.rgb), 1.0 / 2.2);
        sum.rgb *= sum.a;
    
        return sum;
    }



float4 main(PixelInputType input):SV_TARGET
{
    //trivial, correct
        float3 rayOrigin = eyePos.xyz;

    //correct
        float2 ndc = float2((input.tex.x * 2.f - 1.f) * (16.f / 9.f), input.tex.y * 2.f - 1.f);

    //correct however a bit wide for now...
    //focal depth might have to increase as widely cast rays can cause stretching artifacts in corners - MUST SYNC WITH SCENE!
        float3 rayDir = normalize(camMatrix._11_12_13 * ndc.x + camMatrix._21_22_23 * ndc.y + camMatrix._31_32_33 * FOCAL_DEPTH);

    //bottom and top of a single cloud layer... might be faster to use onioning... but this works nicely!
        float t1 = RaySphereInt(rayOrigin, rayDir, PLANET_CENTER, PLANET_RADIUS + CLOUD_BOTTOM).y;
        float t2 = RaySphereInt(rayOrigin, rayDir, PLANET_CENTER, PLANET_RADIUS + CLOUD_TOP).y;

        if (t1 > MAX_VIS)
            return atmosphere(t1);

    //works only for from underneath the layer
        if (t1 > 0. && t2 > 0.)
        {
            rayOrigin += rayDir * (blueNoise.Sample(CloudSampler, ndc).x * 2.f - 1.f); //blue noise
            return raymarch(rayOrigin + rayDir * t1, rayDir, t2 - t1);
        }

        return (float4) 0.;
    }



 //horrible way to do it! assumes perfectly flat terrain... just move the quad far back - @TODO MENTION IN THESIS!
 //float2 tEarth = RaySphereInt(rayOrigin, rayDir, PLANET_CENTER, PLANET_RADIUS);
 //if (max(tEarth.x, tEarth.y) > -0.0001f)
    //return float4(0., 0., 0., 0.);

//interesting yet currently useless section
/*

float2 get_ndc(float2 uv)
{
    uv.y = 1.0f - uv.y;
    float ndc_x = (uv.x * 2.0f) - 1.0f;
    ndc_x *= (16.0f / 9.0f);
    float ndc_y = (uv.y * 2.0f) - 1.0f;
    return float2(ndc_x, ndc_y);
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
*/



/*
float2 get_ndc(float2 uv)
{
    uv.y = 1.0f - uv.y;
    float ndc_x = (uv.x * 2.0f) - 1.0f;
    ndc_x *= (16.0f / 9.0f);
    float ndc_y = (uv.y * 2.0f) - 1.0f;
    return float2(ndc_x, ndc_y);
}
float3 get_ray_dir(float2 ndc, float focal_length, float4x4 transform)
{
    float3 right = transform._11_12_13;
    float3 up = transform._21_22_23;
    float3 forward = transform._31_32_33;

    return normalize((forward * focal_length) + (right * ndc.x) + (up * ndc.y));
}

uint ray_sphere_intersect(float3 ray_origin, float3 ray_dir, float3 sphere_origin, float sphere_radius, out float2 t)
{
    ray_dir = normalize(ray_dir);

    float3 l = ray_origin - sphere_origin;

    float a = 1.0f;
    float b = 2.0f * dot(ray_dir, l);
    float c = dot(l, l) - sphere_radius * sphere_radius;

    float discr = b * b - 4.0f * a * c;

    if (discr < 0.0f)
    {
        t.x = t.y = 0.0f;
        return 0u;
    }
    else if (abs(discr) - 0.00005f <= 0.0f)
    {
        t.x = t.y = -0.5f * b / a;
        return 1u;
    }
    else
    {
        float q = b > 0.0f ?
			-0.5f * (b + sqrt(discr)) :
			-0.5f * (b - sqrt(discr));
        float h1 = q / a;
        float h2 = c / q;
        t.x = min(h1, h2);
        t.y = max(h1, h2);
        if (t.x < 0.0f)
        {
            t.x = t.y;
            if (t.x < 0.0f)
            {
                return 0u;
            }
            return 1u;
        }
        return 2u;
    }
}


bool get_cloud_layer(in float3 eye_pos, in float3 ray_dir, out float3 cloud_layer_start, out float3 cloud_layer_end)
{
    float3 planet_center = PLANET_CENTER;
    float inner_cloud_radius = PLANET_RADIUS + CLOUD_BOTTOM;
    float outer_cloud_radius = PLANET_RADIUS + CLOUD_TOP;

    float eye_to_world_center_distance = distance(eye_pos, planet_center);

    float2 inner_t = 0.0f;
    uint inner_num_intersctions = ray_sphere_intersect(eye_pos, ray_dir, planet_center, inner_cloud_radius, inner_t);

    float2 outer_t = 0.0f;
    uint outer_num_intersctions = ray_sphere_intersect(eye_pos, ray_dir, planet_center, outer_cloud_radius, outer_t);

    float3 inner_cloud_hit = eye_pos + (ray_dir * inner_t.x);
    float3 outer_cloud_hit = eye_pos + (ray_dir * outer_t.x);

    // under the clouds
    if (eye_to_world_center_distance < inner_cloud_radius)
    {
        // below horizon
        if (inner_cloud_hit.y < 0.0f)
        {
            cloud_layer_start = 0.0f;
            cloud_layer_end = 0.0f;
            return false;
        }

        cloud_layer_start = inner_cloud_hit;
        cloud_layer_end = outer_cloud_hit;
        return true;
    }
    // over the clouds
    else if (eye_to_world_center_distance > outer_cloud_radius)
    {
        // not looking down at clouds
        if (outer_num_intersctions == 0U)
        {
            cloud_layer_start = 0.0f;
            cloud_layer_end = 0.0f;
            return false;
        }

        if (inner_num_intersctions == 0U)
        {
            cloud_layer_start = 0.0f;
            cloud_layer_end = 0.0f;
            return false;
        }

        // below horizon
        //if (outer_cloud_hit.y < 0.0f){
        //    cloud_layer_start = 0.0f;
        //    cloud_layer_end = 0.0f;
        //    return false;
        //}

        cloud_layer_start = outer_cloud_hit;
        cloud_layer_end = inner_cloud_hit;
        return true;
    }
    // in between clouds
    else
    {
        if (inner_num_intersctions > 0u)
        {
            cloud_layer_end = inner_cloud_hit;
        }
        else
        {
            cloud_layer_end = outer_cloud_hit;
        }

        cloud_layer_start = eye_pos;
        return true;
    }
}



//Inigo Quilez version... I don't get one part of it...
float IntersectRaySphere(float3 ro, float3 rd, float3 ce, float radius)
{
    float3 oc = ro - ce;    //use this to project it onto (normalized!!!) rd for closest point
    float b = dot(oc, rd);  //closest point to sphere center is along the ray where t = b (projected Sc onto ray...)

    //squared distance between edge of sphere closest to ray origin and ray origin
    float c = dot(oc, oc) - radius * radius; //pythagoras... no sqrt yet! optimized!

    float discriminant = b * b - c; //compare squared distance to closest point and radius
    if (discriminant < 0)       //basically closest point not in sphere so no intersection, avoid sqrt
    {
        return -1.;
    }
    else
    {
        float discriminant = sqrt(discriminant);           //ok this bastard has to happen eventually
        return min(-b - discriminant, -b + discriminant);   //
    }
}
*/