cbuffer VariableBuffer : register(b0)
{
	float elapsed;
	float3 padding;
};


cbuffer ViewDirBuffer : register(b1)
{
	float4x4 rotationMatrix;
	float4 eyePos;
};



struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 msPos : MSPOS;
	float4 wPos : WPOS;
};

Texture2D shaderTexture;
SamplerState SampleType;


float remap(float value, float min1, float max1, float min2, float max2)
{
	return min2 + ((value - min1) / (max1 - min1)) * (max2 - min2);
}

float3 mod289(float3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 mod289(float4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 permute(float4 x) {
	return mod289(((x*34.0) + 1.0)*x);
}

float4 taylorInvSqrt(float4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(float3 v)
{
	const float2  C = float2(1.0 / 6.0, 1.0 / 3.0);
	const float4  D = float4(0.0, 0.5, 1.0, 2.0);

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
	float3  ns = n_ * D.wyz - D.xzx;

	float4 j = p - 49.0 * floor(p * ns.z * ns.z);

	float4 x_ = floor(j * ns.z);
	float4 y_ = floor(j - 7.0 * x_);

	float4 x = x_ * ns.x + ns.yyyy;
	float4 y = y_ * ns.x + ns.yyyy;
	float4 h = 1.0 - abs(x) - abs(y);

	float4 b0 = float4(x.xy, y.xy);
	float4 b1 = float4(x.zw, y.zw);

	float4 s0 = floor(b0)*2.0 + 1.0;
	float4 s1 = floor(b1)*2.0 + 1.0;
	float4 sh = -step(h, float4(0., 0., 0., 0.));

	float4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
	float4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

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
	return 42.0 * dot(m*m, float4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}


//Helpers
static const float PI = 3.141592f;
static const float INTENSITY = 1.61803f * PI;
static const float TWISTER = 5.;

//FBM settings
static const int NUM_OCTAVES = 3;
static const float LACUNARITY = 1.13795;
static const float GAIN = .797531;

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

//here for reference, changed it slightly
float3 opTwist(in float3 p)
{
	float nani = TWISTER * p.y - elapsed;
	float c = cos(nani);
	float s = sin(nani);
	float2x2 rotoMato = float2x2(c, -s, s, c);
	return  float3(mul(p.xz, rotoMato), p.y);
}

float sdTorus(float3 p, float2 t)
{
	return length(float2(length(p.xz) - t.x, p.y)) - t.y;
}

//Raymarch settings
static const int NUM_STEPS = 20;
static const float STEP_SIZE = 2.f / (float)NUM_STEPS;

float4 raymarch(in float3 rayOrigin, in float3 rayDir, in float2x2 rotMat)
{
	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float particle;

	float flame = 0.f;
	float t = 0.f;
	float3 noiseDir = float3(0., .66, .33);

	for (int i = 0; i < NUM_STEPS; ++i)
	{
		if (flame > .99f)	break;

		float3 curPos = rayOrigin + t * rayDir;

		float3 twisted = opTwist(curPos);

		float mask = 1.f - sdTorus(twisted, float2(0.4, 0.05));
		mask *= turbulentFBM(twisted - elapsed);

		mask = pow(mask, 5);

		flame += STEP_SIZE * mask;

		t += STEP_SIZE;
	}

	//sum = smoothstep(float4(0., 0., 0., 0.), float4(1.2, 1.2, 1.2, 1.), float4(.2 * flame , flame, .7f * flame, flame));

	sum = float4(smoothstep(.6, 1., flame), flame, smoothstep(1., .6, flame), flame);

	return sum;
}


float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float3 xyz = input.msPos.xyz;

	float3 viewdir = normalize(input.wPos.xyz - eyePos.xyz); //ray direction

	float nani = TWISTER * xyz.y + elapsed;
	float c = cos(nani);
	float s = sin(nani);
	float2x2 rotMat = float2x2(c, -s, s, c);

	float4 colour = raymarch(xyz, viewdir, rotMat);

	return colour;
}