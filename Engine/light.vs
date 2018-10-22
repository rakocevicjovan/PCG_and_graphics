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
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WNORM;
};


PixelInputType LightVertexShader(VertexInputType input){
   
	PixelInputType output;
    
    input.position.w = 1.0f;

    output.worldPos = mul(input.position, worldMatrix);	//careful... doing this to optimize and avoid copying
    output.position = mul(output.worldPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
	// Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);		//transpose(inverse((float3x3)worldMatrix)) with non-uniform scaling
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    return output;
}