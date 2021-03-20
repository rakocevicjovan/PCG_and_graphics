cbuffer PSPerCameraBuffer : register(b9)
{
	matrix _projection;
	float scr_w;
	float scr_h;
	float zNear;
	float zFar;
};


cbuffer PSPerFrameBuffer : register(b10)
{
	float4 eyePos;
	float elapsed;
	float delta;
	float2 padding;
}