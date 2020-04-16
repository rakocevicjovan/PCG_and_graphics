cbuffer PSPerFrameBuffer : register(b10)
{
	float4 eyePos;
	float elapsed;
	float delta;
	float2 padding;
}


struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float depth : ZDEPTH;	// 0-1 range, projected, no perspective divide
};


#define ONLYZ
float4 main(PixelInputType input) : SV_TARGET
{
	float4 output = (float4) 0.f;

	float realDepth = (input.depth) * (1000.f - 0.1f);	// not really but ok for this
	float sliceThicc = (1000.f - 0.1f) / 16.f;

#ifdef XYZ
	output.r = ((int)input.position.x >> 6) / 30.f;			// divide into 30 slices of 64 pixels
	output.g = ((int)input.position.y / 60) / 18.f;			// divide into 18 slices of 60 pixels
	output.b = (realDepth / sliceThicc)		/ 16.f;			// divide into 16 slices according to depth
#endif

#ifdef ONLYZ
	float slice = (realDepth / sliceThicc);
	//output.r = (slice % 3) - 1;
	//output.g = ((slice + 1) % 3) - 1;
	//output.b = ((slice + 2) % 3) - 2;
	output.b = smoothstep(0, 1, slice / 16.f);

#endif

	output.w = 1.f;

	return output;
}