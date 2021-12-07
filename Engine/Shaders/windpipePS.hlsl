#include "Noise.hlsli"
#include "SDF.hlsli"

cbuffer VolumetricScreenBuffer : register(b0)
{
    float4 cameraPosition;  //and time elapsed for w
    float4 cameraForward;
    float4 gale1;           //xz of tube, height, radius
    float4 gale2;
    float4 gale3;
    float4 gale4;
};

#define elapsed cameraPosition.w

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 wPos : WPOS;
};


//Constants
//FBM settings
static const float LACUNARITY = 3.141592f;	// just PI really
static const float GAIN = .79;
static const int NUM_OCTAVES = 5;
//Raymarch settings
static const int STEPS = 100;
static const float MAX_DIST = 1000.f;
static const float EPS = 0.0001f;


//Windpipes (actually capsules in shape)
struct Windpipe
{
    float3 a;
    float3 b;
    float rad;
};

Windpipe makeWindpipe(float4 gale)
{
    Windpipe w;
    w.a = float3(gale.x, 200, gale.z);
    w.b = float3(gale.x, gale.y, gale.z);
    w.rad = gale.w;
    return w;
}

float2 myRot(float2 k, float t)
{
    float ct = cos(t);
    float st = sin(t);
    return float2(ct * k.x - st * k.y, st * k.x + ct * k.y);
}



float ridged(float f)
{
    return 1.0 - 2.0 * abs(f);
}


//c is windpipes xzw, aka horizontal position and radius
float sdTornado(float3 p, float3 c, float bottom, float top)   //expanded to look like a tornado
{
    float heightScale = smoothstep(bottom, top, p.y) * 2.f + .1f;

    float radius = c.z;
    radius *= heightScale;
    
    float2 center = c.xy;
    center += myRot(float2(sin(elapsed), 33), elapsed) * heightScale; //+ sin(heightScale - elapsed) * 33;

    float distToCenter = length(p.xz - center) - radius;

    //distToCenter = p.y < bottom ? 100000 : distToCenter;
    //return p.y > top ? 100000 : distToCenter;
    return distToCenter;
}



float4 getCol(in float3 rayOrigin, in float3 rayDir)
{
    float colour = 0.f;
    for (int i = 0; i < 24; ++i)
    {
        float3 curPos = rayOrigin + i * rayDir;

        float3 adjustedPos = curPos - float3(0, elapsed * 10.f, 0);
        adjustedPos.xz = myRot(adjustedPos.xz, elapsed) * .01f;
        adjustedPos.y -= 3. * elapsed;
        adjustedPos.y *= .1;

        colour += fbm(adjustedPos, LACUNARITY, GAIN, NUM_OCTAVES) * 24.;
    }
  
    //colour = smoothstep(.6, 1.8, colour);
    float b = colour;
    float g = smoothstep(.3, 1., colour);
    return float4(b * b, g, b, min(b, g));
}



float4 raymarch(in float3 rayOrigin, in float3 rayDir, Windpipe wp)
{
    float4 sum = (float4) 0;
    float3 curPos = rayOrigin;

    float mask = 0.f;
    float totalDist = 0;

    for (int i = 0; i < STEPS; ++i)
    {
        curPos = rayOrigin + totalDist * rayDir;

        float tornadoDist = sdTornado(curPos, float3(wp.a.xz, wp.rad), wp.a.y, wp.b.y);
        totalDist += tornadoDist;

        if (totalDist > MAX_DIST || tornadoDist < EPS)
            break;
    }

    mask = (1.f - totalDist / MAX_DIST);

    if (mask < 0.0001f)
        return sum;

    sum = getCol(curPos, rayDir);

    return sum;
}



float4 main(PixelInputType input) : SV_TARGET
{
    float3 camPos = cameraPosition.xyz;

    float3 viewDir = normalize(input.wPos.xyz - camPos);
    
    //try just one gale first - capsule shape defined there!
    Windpipe wp1 = makeWindpipe(gale1);
    Windpipe wp2 = makeWindpipe(gale2);
    Windpipe wp3 = makeWindpipe(gale3);
    Windpipe wp4 = makeWindpipe(gale4);

    float4 colour;
    colour = raymarch(camPos, viewDir.xyz, wp1);
    colour = max(raymarch(camPos, viewDir.xyz, wp2), colour);
    colour = max(raymarch(camPos, viewDir.xyz, wp3), colour);
    colour = max(raymarch(camPos, viewDir.xyz, wp4), colour);

    return colour;
}