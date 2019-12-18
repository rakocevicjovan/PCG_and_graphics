struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

TextureCube cubeMapTexture : register(t0);
SamplerState Sampler : register(s0);




float N2(float2 p)
{ // Dave Hoskins - https://www.shadertoy.com/view/4djSRW
    float3 p3 = frac(float3(p.xyx) * float3(443.897, 441.423, 437.195));
    p3 += dot(p3, p3.yzx + 19.19);
    return frac((p3.x + p3.y) * p3.z);
}



float2 randVec2(float2 p)
{
    return float2(N2(p), N2(p.yx)) - .5;
}



float2 getCellStar(float2 cellID)
{
    //float t = iTime * .33;      //return vec2(sin(t * temp.x), cos(t * temp.y)) * .4; //moving
    float2 temp = randVec2(cellID);
    return float2(temp.x, temp.y) * .8;
}






float3 getSpherical(float3 inPos)
{
    float3 result;
    result.x = length(inPos);
    result.y = atan2(inPos.x, inPos.y);
    result.z = acos(inPos.z / result.x);
    return result;
}



float3 gridSpherical(float3 inSph, float d1, float d2)
{
    return float3(inSph.x, floor(inSph.y * d1), floor(inSph.z * d2));
}



float3 getCartiesan(float3 inSph)
{
    return float3(
    inSph.x * cos(inSph.y) * sin(inSph.z),
    inSph.x * sin(inSph.y) * sin(inSph.z),
    inSph.x * cos(inSph.z));
}



float4 main(PixelInputType input) : SV_TARGET
{
    /*
    float3 spPos = getSpherical(-input.normal);
    float3 wat = gridSpherical(spPos, 20., 20.);
    float3 cart = getCartiesan(wat);
    float2 uv = cart.xy;

    float4 colour = (float4) 0.;

    float2 gridCell = floor(uv);
    float2 gridOffset = frac(uv) - .5;

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {

            float2 adjCell = gridCell + float2(i, j);
            float2 adjCellPoint = getCellStar(adjCell);
            float2 adjGridOffset = gridOffset - float2(i, j);
            
            float2 rekt = adjGridOffset - adjCellPoint;
            float dToAdj = length(rekt);
            
            colour.xyz += (float3)smoothstep(.4, .1, dToAdj);
        }
    }
    */

    return cubeMapTexture.Sample(Sampler, input.texCoord);
}
