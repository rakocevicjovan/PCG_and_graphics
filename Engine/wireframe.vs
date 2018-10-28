/*
#version 400

uniform mat4 MVP;
uniform mat4 M;

in layout(location = 0) vec3 vertexPosition;

out vec4 wPos;

void main(){

    gl_Position = MVP * vec4(vertexPosition, 1.0f);
	wPos = M * vec4(vertexPosition, 1.0f);

}
*/

///////////////////////////////////////////////////////

cbuffer MatrixBuffer{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInputType{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
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