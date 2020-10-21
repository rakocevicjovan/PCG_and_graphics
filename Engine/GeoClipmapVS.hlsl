cbuffer PerCameraBuffer : register(b10)
{
	matrix projectionMatrix;
};

cbuffer PerFrameBuffer : register(b11)
{
	matrix viewMatrix;
	float dTime;
	float eTime;
	float2 padding;
};

cbuffer GeoClipmapBuffer : register(b0)
{
	float4 scaleTranslation;
	float4 hmapScaleTranslation;	// 1/(w, h) of texture,  xy origin of block in texture
	float4 stuff;
	// Some packing BS is happening and it pads the whole cb to 64 bytes which makes no sense
	//float zScale;
	//float zTexScale;
	//float alphaOffset;				// = ((n - 1) / 2 - w - 1); Precalc this
	//float transitionWidth;
};

Texture2D<float> heightMap : register(t0);

struct VsIn
{
	float2 xzPos : POSITION;
};

struct VSOut
{
	float4 position : SV_POSITION;

	// Normal map lookup coordinates
	float2 uv : TEXCOORD0;

	// Transition blend on normal map
	float alpha : TEXCOORD1;

	// Elevation map lookup coordinate - it's used by the paper for elevation based colour from a 1D texture 
	float z : TEXCOORD2;
	
	//float3 normal : NORMAL;	// Geo Clipmapping fully relies on a normal map since height isn't known aot
};

VSOut main(VsIn input)
{
	VSOut output;

	output.position = float4(input.xzPos.x, 0., input.xzPos.y, 1.);

	/*
	float2 uv = input.xzPos * hmapScaleTranslation.xy + hmapScaleTranslation.zw;
	float2 worldPos = input.xzPos * scaleTranslation.xy + scaleTranslation.zw;

	// Compact into a single float4 register
	float zf_zd = heightMap.Load(float3(uv, 0));
	float zf = floor(zf_zd);
	float zd = frac(zf_zd) * 512 - 256; // Encode zd as: zd = zc - zf

	
	// n is clipmap grid range, w is transition width (paper used n/10), precalc 1/transitionWidth and alphaOffset in cbuffer 
	float2 vxy = float2(viewMatrix._41, viewMatrix._42);
	float2 axy = clamp((abs(worldPos - vxy) - stuff.z) * (1. / stuff.w), 0, 1);
	axy.x = max(axy.x, axy.y);	// Reuse for less occupancy

	float height = zf + axy.x * zd;		//lerp(zf, zc, axy.x); would be the same thing, but we have zd not zc
	height *= stuff.x;

	output.position = float4(worldPos.x, height, worldPos.y, 1.);
	output.uv = uv;
	output.z = output.position.z * stuff.y;	// Not really required, I don't need height based colour
	output.alpha = axy.x;
	*/
	return output;
}