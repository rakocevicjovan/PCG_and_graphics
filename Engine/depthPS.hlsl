struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;	// Do I reaaaally need this? Think not!
};



float4 main(PixelInputType input) : SV_TARGET
{
	float depthValue = input.depthPosition.z / input.depthPosition.w;
	float4 color = float4(depthValue, depthValue, depthValue, 1.0f);
	return color;
}