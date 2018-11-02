cbuffer MatrixBuffer{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};

cbuffer LightBuffer2{
    float4 lightPosition;
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
	float4 worldPosition : REKT;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};



PixelInputType ShadowVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    output.worldPosition = mul(input.position, worldMatrix);
    output.position = mul(output.worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Calculate the position of the vertices as viewed by the light source.
    output.lightViewPosition = mul(input.position, worldMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);

    output.tex = input.tex;
    
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    output.lightPos = lightPosition.xyz - output.worldPosition.xyz;
    output.lightPos = normalize(output.lightPos);

    return output;
}