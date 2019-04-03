cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


cbuffer VariableBuffer : register(b1)
{
	float4 playerPos;
};


struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float3 tangent : TANGENT;
};


PixelInputType LightVertexShader(VertexInputType input)
{
	PixelInputType output;

	output.worldPos = mul(input.position, worldMatrix);

	//float3 wMatPos = float3(worldMatrix._41, worldMatrix._42, worldMatrix._43);	works!, just don't want it for the maze, but later for the fire level
	float distance = length(playerPos.xz - output.worldPos.xz);//wMatPos.xz);

	output.worldPos.y -= smoothstep(48.f, 80.f, distance) * 27.f;
	output.position = mul(output.worldPos, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;

	output.normal = mul(input.normal, (float3x3)worldMatrix);		//transpose(inverse((float3x3)worldMatrix)) with non-uniform scaling
	output.normal = normalize(output.normal);

	output.tangent = mul(input.tangent, worldMatrix);

	return output;
}