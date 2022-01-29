#include "Constants.hlsli"
#include "Clustering.hlsli"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float depth : ZDEPTH;	// 0-1 range, projected, no perspective divide
};


//#define Z_VS_LINEAR
//#define Z_ORIGINAL
#define Z_DOOM



float4 main(PixelInputType input) : SV_TARGET
{
	float4 output = (float4) 0.f;

	const float n = 1.f;
	const float f = 1000.f;

	const float radFoV = PI / 3.f;

	const float Sx = 30.f;
	const float Sy = 18.f;	// Number of slices along y
	const float Sz = 16.f;

	const float zSpan = f - n;
	const float sliceThicc = zSpan / 16.f;

	float linearDepth = zToViewSpace(input.position.z, n, f);

#ifdef Z_VS_LINEAR
	int slice = linearDepth / sliceThicc;										// Linear slicing using the view depth
#endif

#ifdef Z_ORIGINAL
	int slice = (log(linearDepth / n)) / (log(1.f + 2.f * tan(radFoV) / Sy));	// Original clustered shading paper slicing
#endif

#ifdef Z_DOOM
	int slice = log(linearDepth) * Sz / log(f / n) - Sz * log(n) / log(f / n);	// Tiago Sousa, DOOM 2016
#endif

	output.r = ((int)input.position.x >> 6) / 30.f;			// divide into 30 slices of 64 pixels
	output.g = ((int)input.position.y / 60) / 18.f;			// divide into 18 slices of 60 pixels
	output.b = slice / 16.f;

	output.w = 1.f;

	return output;
}