cbuffer LightBuffer
{
    float3 alc;
    float ali;
    float3 dlc;
    float dli;
    float3 slc;
    float sli;
    float4 lightPosition;
    float4 eyePos;
};


struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 worldPos : WPOS;
};

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
SamplerState Sampler;

static const float SpecularPower = 8.f;
float4 calcAmbient(in float3 alc, in float ali)
{
    return saturate(float4(alc, 1.0f) * ali);
}
float4 calcDiffuse(in float3 invLightDir, in float3 normal, in float3 dlc, in float dli, inout float dFactor)
{
    dFactor = max(dot(normal, invLightDir), 0.0f);
    return saturate(float4(dlc, 1.0f) * dli * dFactor);
}
float4 calcSpecular(in float3 invLightDir, in float3 normal, in float3 slc, in float sli, in float3 invViewDir, in float dFactor)
{
    float3 reflection = normalize(reflect(invLightDir, normal));
    float sFactor = pow(saturate(dot(reflection, invViewDir)), SpecularPower);
    return saturate(float4(slc, 1.0f) * sFactor * sli * dFactor);
}


// fast 3d noise by Inigo Quilez
float noise(in float3 x) //3d noise from iq
{
    float3 p = floor(x);
    float3 f = frac(x);
    f = f * f * (3.0 - 2.0 * f);
    float2 uv = (p.xy + float2(37.0, 17.0) * p.z) + f.xy;
    float2 rg = tex0.Sample(Sampler, (uv + 0.5) / 256.0).yx;
    return lerp(rg.x, rg.y, f.z);
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

	// First corner
    float3 i = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

	// Other corners
    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
    float3 x1 = x0 - i1 + C.xxx;
    float3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    float3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

	 // Permutations
    i = mod289(i);
    float4 p = permute(permute(permute(i.z + float4(0.0, i1.z, i2.z, 1.0)) + i.y + float4(0.0, i1.y, i2.y, 1.0)) + i.x + float4(0.0, i1.x, i2.x, 1.0));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    float3 ns = n_ * D.wyz - D.xzx;

    float4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,7*7)

    float4 x_ = floor(j * ns.z);
    float4 y_ = floor(j - 7.0 * x_); // mod(j,N)

    float4 x = x_ * ns.x + ns.yyyy;
    float4 y = y_ * ns.x + ns.yyyy;
    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

	//float4 s0 = float4(lessThan(b0,0.0))*2.0 - 1.0;
	//float4 s1 = float4(lessThan(b1,0.0))*2.0 - 1.0;
    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, float4(0., 0., 0., 0.));

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 p0 = float3(a0.xy, h.x);
    float3 p1 = float3(a0.zw, h.y);
    float3 p2 = float3(a1.xy, h.z);
    float3 p3 = float3(a1.zw, h.w);

	//Normalise gradients
    float4 norm = taylorInvSqrt(float4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

	// Mix final noise value
    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, float4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}



#define OCTAVES 8
#define GAIN .79
#define LACUNARITY 3.14159

float turbulentFBM(float3 x)
{
    float sum = 0.0f;

    float frequency = 2.0f;
    float amplitude = 1.0f;

    for (int i = 0; i < OCTAVES; ++i)
    {
        float r = snoise(frequency * x) * amplitude;
        sum += r;
        frequency *= LACUNARITY;
        amplitude *= GAIN;
    }

    return sum;
}



#define STEPS 12
#define STEP_SIZE 1.f / (float)STEPS

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    input.normal = normalize(input.normal);

    float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);

    float3 viewDir = input.worldPos.xyz - eyePos.xyz;
    float distance = length(viewDir);
    viewDir = viewDir / distance;
    float3 invViewDir = -viewDir;

    float4 colour = tex0.Sample(Sampler, input.tex);
    colour += turbulentFBM(input.worldPos.xyz / 10.f);

    float4 ambient = calcAmbient(alc, ali);
    float dFactor = 0;
    float4 diffuse = calcDiffuse(-lightDir, input.normal, dlc, dli, dFactor);
    float4 specular = calcSpecular(-lightDir, input.normal, slc, sli, viewDir, dFactor);
    colour.xyz = (ambient.xyz + diffuse.xyz) * colour.xyz + specular.xyz;
    colour.xyz = pow(colour.xyz, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
 
    return colour;
}


/*float4 colour = (float4) 0.0f;

float r = (1.f - dot(input.normal, invLightDir)) * STEP_SIZE;

for (int i = 0; i < STEPS; ++i)
{
    float3 curPos = input.worldPos.xyz + i * STEP_SIZE * viewDir;

    float g = turbulentFBM(curPos / 33.f) * STEP_SIZE + r * .2f;
    float b = .6f - max(r, g);
    colour.rg += float2(r, g);
    colour.b += b * STEP_SIZE;
    colour.a += max(g, b);
}*/