#include "Simplex.hlsli"

// fast 3d noise by Inigo Quilez
float noiseIQ(in float3 x) //3d noise from iq
{
	float3 p = floor(x);
	float3 f = frac(x);
	f = f * f * (3.0 - 2.0 * f);
	float2 uv = (p.xy + float2(37.0, 17.0) * p.z) + f.xy;
	float2 rg = tex0.Sample(Sampler, (uv + 0.5) / 256.0).yx;
	return lerp(rg.x, rg.y, f.z);
}



float turbulentFBM(in float3 x, uniform float lacunarity, uniform float gain, uniform int nOctaves)
{
	float sum = 0.0f;

	float frequency = 1.0f;
	float amplitude = 1.0f;

	for (int i = 0; i < nOctaves; ++i)
	{
		float r = snoise(frequency * x) * amplitude;
		r = r < 0 ? -r : r;
		sum += r;
		frequency *= lacunarity;
		amplitude *= gain;
	}

	return sum;
}



float fbm(in float3 pos, uniform float lacunarity, uniform float gain, uniform int nOctaves)
{
	float sum = 0.0;

	float amplitude = 1.f;
	float frequency = 1.f;

	for (int i = 0; i < nOctaves; ++i)
	{
		sum += snoise(frequency * pos) * amplitude;
		frequency *= lacunarity;
		amplitude *= gain;
	}
	return sum;
}