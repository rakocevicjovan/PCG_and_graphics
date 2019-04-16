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


float remap(float value, float min1, float max1, float min2, float max2)
{
    return min2 + ((value - min1) / (max1 - min1)) * (max2 - min2);
}

float3 mod289(float3 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 mod289(float4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 permute(float4 x)
{
    return mod289(((x * 34.0) + 1.0) * x);
}

float4 taylorInvSqrt(float4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(float3 v)
{
    const float2 C = float2(1.0 / 6.0, 1.0 / 3.0);
    const float4 D = float4(0.0, 0.5, 1.0, 2.0);

    float3 i = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);

    float3 x1 = x0 - i1 + C.xxx;
    float3 x2 = x0 - i2 + C.yyy;
    float3 x3 = x0 - D.yyy;

    i = mod289(i);
    float4 p = permute(permute(permute(i.z + float4(0.0, i1.z, i2.z, 1.0)) + i.y + float4(0.0, i1.y, i2.y, 1.0)) + i.x + float4(0.0, i1.x, i2.x, 1.0));

    float n_ = 0.142857142857;
    float3 ns = n_ * D.wyz - D.xzx;

    float4 j = p - 49.0 * floor(p * ns.z * ns.z);

    float4 x_ = floor(j * ns.z);
    float4 y_ = floor(j - 7.0 * x_);

    float4 x = x_ * ns.x + ns.yyyy;
    float4 y = y_ * ns.x + ns.yyyy;
    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, float4(0., 0., 0., 0.));

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 p0 = float3(a0.xy, h.x);
    float3 p1 = float3(a0.zw, h.y);
    float3 p2 = float3(a1.xy, h.z);
    float3 p3 = float3(a1.zw, h.w);

    float4 norm = taylorInvSqrt(float4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, float4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}


//Helpers
static const float PI = 3.141592f;

//FBM settings
static const int NUM_OCTAVES = 5;
static const float LACUNARITY = PI;
static const float GAIN = .79;

//Raymarch settings
static const int STEPS = 100;
static const float MAX_DIST = 1000.f;
static const float EPS = 0.0001f;


float turbulentFBM(float3 x)
{
    float sum = 0.0f;

    float frequency = 1.0f;
    float amplitude = 1.0f;

    for (int i = 0; i < NUM_OCTAVES; ++i)
    {
        float r = snoise(frequency * x) * amplitude;
        //r = abs(r);
        sum += r;
        frequency *= LACUNARITY;
        amplitude *= GAIN;
    }

    return sum;
}


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
float sdCylinder(float3 p, float3 c, float bottom, float top)   //expanded to look like a tornado
{
    float heightScale = smoothstep(bottom, top, p.y) * 2.f + .1f;

    float radius = c.z;
    radius *= heightScale;
    
    float2 center = c.xy;
    center += myRot(float2(sin(elapsed), 33), elapsed) * heightScale;

    float distToCenter = length(p.xz - center) - radius;

    distToCenter = p.y < bottom ? 100000 : distToCenter;

    return p.y > top ? 100000 : distToCenter;
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

        colour += turbulentFBM(adjustedPos) / 24.;
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

        float tornadoDist = sdCylinder(curPos, float3(wp.a.xz, wp.rad), wp.a.y, wp.b.y);
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



float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float3 camPos = cameraPosition.xyz;

    float2 uv = input.position.xy / float2(1600, 900);
    uv *= 2.f;
    uv -= 1.f;

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