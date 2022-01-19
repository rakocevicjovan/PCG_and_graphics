struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : colour;
};

float4 main(VSOut PSIn) : SV_TARGET
{
    return float(1.f);
    // Visualize 10x10 grid over the triangle
    //return float4(fmod(PSIn.uv, .1), 0., 1.);
}