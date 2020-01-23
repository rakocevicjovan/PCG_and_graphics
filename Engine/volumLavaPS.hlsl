#include "Simplex.hlsli"
#include "Noise.hlsli"
#include "MathUtils.hlsli"

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

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

float2x2 otherMat = float2x2(0.80, 0.60, -0.60, 0.80);

float grid(float2 p)
{
	float s = sin(p.x)*cos(p.y);
	return s;
}


float flow(in float2 p)
{
	float z = 2.;
	float rz = 0.;
	float2 bp = p;
	for (float i = 1.; i < 7.; i++)
	{
		bp += elapsed * 1.5;
		float2 gr = float2(grid(p * 3. - elapsed * 2.), grid(p * 3. + 4. - elapsed * 2.)) * 0.4;
		gr = normalize(gr) * 0.4;
		gr = mul(gr, makeRotMat((p.x + p.y) *.3 + elapsed * 10.));
		p += gr * 0.5;

		rz += (sin(snoise(float3(p.x, 0, p.y) * 8.)) * 0.5 + 0.5) / z;

		p = lerp(bp, p, 0.5);
		z *= 1.7;
		p *= 2.5;
		p = mul(p, otherMat);
		bp *= 2.5;
		bp = mul(bp, otherMat);
	}
	return rz;
}


static const float LACUNARITY = 1.9357f;
static const float GAIN = .5317f;
static const int NUM_OCTAVES = 10;

static const float LOWER = .4;
static const float UPPER = 1.2;


float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float3 xyz = float3(input.wPos.x, input.wPos.y, input.wPos.z);

	xyz.xz -= 512;
	xyz.y -= elapsed * 5.f;		//move the texture upwards

	float3 scaledPos = xyz * .05f;

	float r = turbulentFBM(scaledPos, LACUNARITY, GAIN, NUM_OCTAVES);
	
	float g = smoothstep(LOWER * 2.f, UPPER, r);
	r = smoothstep(LOWER, UPPER, r);

	float4 colour = { r, g, 0., 1.f };

	return colour;
}