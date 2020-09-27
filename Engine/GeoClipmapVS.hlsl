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

Texture2D<float> heightMap : register(t0);

cbuffer GeoClipmapBuffer : register(b0)
{
	float4 scaleTranslation;
	float4 hmapScaleTranslation;	// 1/(w, h) of texture,  xy origin of block in texture
	float zScale;
	float zTexScale;
	float alphaOffset;				// = ((n - 1) / 2 - w - 1); Precalc this
};

struct VsIn
{
	float2 xzPos;
};

struct VsOut
{
	float4 position : SV_POSITION;

	// Normal map lookup coordinates
	float2 uv;

	// Transition blend on normal map
	float alpha;

	// Elevation map lookup coordinate - it's used by the paper for elevation based colour from a 1D texture 
	float z;
	
	//float3 normal : NORMAL;	// Geo Clipmapping fully relies on a normal map since height isn't known aot
};

VsOut main(VsIn input)
{
	VSOut output;

	float2 uv = gridPos * hmapScaleTranslation.xy + hmapScaleTranslation.zw
	float2 worldPos = input.xzPos.xy * scaleTranslation.xy + scaleTranslation.zw;

	// Compact into a single float4 register
	float zf_zd = heightMap.Load(uv);
	float zf = floor(zf_zc);
	float zd = frac(zf_zd) * 512 - 256; // (zd = zc - zf)

	
	// n is clipmap grid range, w is transition width (paper used n/10), precalc 1/w and alphaOffset in cbuffer 
	float2 vxy = float2(viewMatrix._41, viewMatrix._42);
	float2 axy = clamp((abs(worldPos - vxy) - alphaOffset) * (1. / w), 0, 1);
	axy.x = max(axy.x, axy.y);	// Reuse for less occupancy

	float height = zf + axy.x * zd;		//lerp(zf, zc, axy.x); would be the same thing, but we have zd not zc
	height *= zScale;

	output.position = float4(worldPos.x, height, worldPos.y, 1.);
	output.uv = uv;
	output.z = z * zTexScale;	// Not really required, I don't need height based colour
	output.alpha = alpha.x;

	return output;
}