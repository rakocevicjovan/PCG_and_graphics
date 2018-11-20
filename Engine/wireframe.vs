cbuffer MatrixBuffer{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInputType{
    float4 position : POSITION;
    float2 tex : TEXCOORD;
	float3 normal : NORMAL;
};

struct GeomInputType{
    float4 position : SV_POSITION;
	float4 worldPos : WPOS;
};

GeomInputType WFVS(VertexInputType input){
   
   	GeomInputType output;

    output.worldPos = mul(input.position, worldMatrix);	//careful... doing this to optimize and avoid copying
    output.position = mul(output.worldPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    return output;
}