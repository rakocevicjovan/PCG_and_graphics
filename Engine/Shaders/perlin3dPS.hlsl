#include "Light.hlsli"
#include "Noise.hlsli"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float time : MYTIME;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

static const float LACUNARITY = 1.9357f;
static const float GAIN = .5317f;
static const int NUM_OCTAVES = 5;


float4 main(PixelInputType input) : SV_TARGET
{
	//can use texcoords for this would be way better...
	input.worldPos = (input.worldPos / 256.f * 2.f) - 1.0f;	//moves it to [0, 1], then to [0, 2] and finally to [-1, 1]

	float x = input.worldPos.x;
	float y = input.worldPos.y;
	float z = input.worldPos.z;		//increase z based on x to split the flame in a bowtie (this means dividing by abs(x) because x -> [-1, 1]

	float3 xyz = float3(x, y + input.time * 0.5f, z);
	float mainTurbulence = turbulentFBM(xyz, LACUNARITY, GAIN, NUM_OCTAVES);
	float smallTurbulence = turbulentFBM(float3(xyz.x * 10.f, xyz.y, xyz.z * 5.f), LACUNARITY, GAIN, NUM_OCTAVES);
	
	float inverseHeight = (1.f - z) * 0.66f;	//(1.f - z) * 0.66f for pointy tip	//or use abs(z) for a diamond shape
	
	float displacement = pow(smallTurbulence, 2) * 0.3f  * smoothstep(0.f, 1.f, z);

	//vary x based on time (in a cycle) to give a more lively flame, where 2.f controls the cycling speed, cap the influence to 10% of the size using * 0.1f
	//divide by inverse height - at z = 1, 1-z becomes 0 therefore x becomes huge (and the flame tapers to a point)
	//modify inverse height by a small displacement, that also depends on z (maps z to 0-1 range, and increases the density as z increases, making tips of flames dense)
	float dist = pow(x * (1.f + sin(input.time * 2.f) * 0.25f) / (inverseHeight * (1.f + displacement)), 2) + pow(z, 2) ;
	float ratio = smoothstep(0., 1., 1.f - dist);


	float r = ratio * mainTurbulence;
	float g = z * ratio;
	float b = min(pow(-z, 3), 1.f - r);
	float4 colour = float4(r, g, b, r);

	return colour;
}