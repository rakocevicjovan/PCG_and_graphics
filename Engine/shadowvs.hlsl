cbuffer MatrixBuffer{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};

cbuffer LightBuffer2{
    float3 lightPosition;
	float padding;
};

struct VertexInputType{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	float4 worldPosition : TEXTURE;
    float4 fragPosLightSpace : TEXCOORD1;
};



PixelInputType ShadowVertexShader(VertexInputType input){

    PixelInputType output;
    
    input.position.w = 1.0f;

    output.worldPosition = mul(input.position, worldMatrix);
    output.position = mul(output.worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Calculate the position of the vertices as viewed by the light source.
    output.fragPosLightSpace = mul(output.worldPosition, lightViewMatrix);
    output.fragPosLightSpace = mul(output.fragPosLightSpace, lightProjectionMatrix);

    output.tex = input.tex;
    output.normal = normalize(mul(input.normal, (float3x3)worldMatrix));	//output.normal = (output.normal);

    return output;
}