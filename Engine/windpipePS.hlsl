cbuffer VolumetricScreenBuffer : register(b0)
{
    float4 cameraPosition;  //and time elapsed for w
    float4 cameraForward;
    float4 gale1;           //xz of tube, height, radius
    float4 gale2;
    float4 gale3;
    float4 gale4;
};



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
static const float TWISTER = 5.;

//FBM settings
static const int NUM_OCTAVES = 3;
static const float LACUNARITY = 1.13795;
static const float GAIN = .797531;

//Raymarch settings
static const int STEPS = 100;
static const float MAX_DIST = 1000.f;
static const float EPS = 0.0001f;

float elapsed;


float turbulentFBM(float3 x)
{
    float sum = 0.0f;

    float frequency = 1.0f;
    float amplitude = 1.0f;

    for (int i = 0; i < NUM_OCTAVES; ++i)
    {
        float r = snoise(frequency * x) * amplitude;
        r = r < 0 ? -r : r;
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
    float r;
};


Windpipe makeWindpipe(float3 ia, float3 ib, float ir)
{
    Windpipe w;
    w.a = ia;
    w.b = ib;
    w.r = ir;
    return w;
}

static const Windpipe windPipe = makeWindpipe((float3) 0, float3(0, 100, 0), 20);


float3 opTwist(in float3 p)
{
    float nani = TWISTER * p.y - elapsed;
    float c = cos(nani);
    float s = sin(nani);
    float2x2 rotoMato = float2x2(c, -s, s, c);
    return float3(mul(p.xz, rotoMato), p.y);
}

float sdTorus(float3 p, float2 t)
{
    return length(float2(length(p.xz) - t.x, p.y)) - t.y;
}

float sdCapsule(float3 p, float3 a, float3 b, float r)
{
    float3 ap = p - a;
    float3 ab = b - a;
    float t = clamp((dot(ap, ab) / dot(ab, ab)), 0., 1.);
    float3 c = a + t * ab;
    return distance(p, c) - r;
}



float4 raymarch(in float3 rayOrigin, in float3 rayDir, Windpipe wp, float elapsed)
{
    float4 sum = (float4) 0;
    float3 curPos = rayOrigin;

    float density = 0.f;
    float totalDist = 0;

    for (int i = 0; i < STEPS; ++i)
    {
        float capsuleDist = sdCapsule(opTwist(curPos), wp.a, wp.b, wp.r);
        totalDist += capsuleDist;
        curPos = rayOrigin + totalDist * rayDir;

        if(totalDist > MAX_DIST || capsuleDist < EPS)
            break;
    }

    density = (1.f - totalDist / MAX_DIST);

    if(density < 0.0001f)
        return sum;

    float3 adjustedPos = curPos - float3(0, elapsed * 20.f, 0);
    float colour = turbulentFBM(adjustedPos / 20.f) * density;

    float b = colour * colour;
    float g = smoothstep(.3, 1., colour);
    sum = float4(0, g, b, min(b, g));
    return sum;
}



float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float elapsed = cameraPosition.w;
    float3 camPos = cameraPosition.xyz;

    float2 uv = input.position.xy / float2(1600, 900);
    uv *= 2.f;
    uv -= 1.f;

    float3 viewDir = normalize(input.wPos.xyz - camPos);
    
    //try just one gale first - capsule shape defined there!
    //Windpipe wp1 = makeWindpipe(float3(gale1.x, 0, gale1.z), float3(gale1.x, gale1.y, gale1.z), gale1.w);
    Windpipe wp1 = makeWindpipe(float3(0, 0, 100), float3(0, 100, 100), 20);

    float4 colour;
    colour = raymarch(camPos, viewDir.xyz, wp1, elapsed);

    return colour;
}