cbuffer MatrixBuffer{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


cbuffer VariableBuffer{
    float delta;
	float3 padding;
};


struct VertexInputType{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
};

static float4x4 identityMatrix =
{
    { 1, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 0, 1 }
};

PixelInputType CMVS(VertexInputType input){
   
	PixelInputType output;

    output.worldPos = mul(input.position, worldMatrix);	//careful... doing this to optimize and avoid copying
    output.position = mul(output.worldPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	output.texCoord = input.position;

    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    return output;
}