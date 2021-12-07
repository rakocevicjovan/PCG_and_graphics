cbuffer PerCameraBuffer : register(b10)
{
	matrix projectionMatrix;
	float scr_w;
	float scr_h;
	float zNear;
	float zFar;
};

cbuffer PSPerFrameBuffer : register(b11)
{
	float4 eyePos;
	float delta;
	float elapsed;
	float2 padding;
}