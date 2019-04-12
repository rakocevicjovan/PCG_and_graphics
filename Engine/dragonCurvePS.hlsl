cbuffer VariableBuffer : register(b1)
{
	float elapsed;
	float3 padding;
};


struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 msPos : MSPOS;
	float4 wPos : WPOS;
};

static const float invSqrt2 = 0.707107;	//same as sin 45 and cos45

static const float si = 0.707107;
static const float co = 0.707107;
static const float2x2 rotMat1 = { co, si, -si, co };

static const float co2 = -0.707107;
static const float2x2 rotMat2 = { co2, si, -si, co2 };

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 colour = float4(0., 0., 0., 0.);

	float f1x = invSqrt2 * mul(input.msPos.xy, rotMat1);
	float f2x = invSqrt2 * mul(input.msPos.xy, rotMat2) + float2(1.0, 0.0);

	colour = (f1x, f2x, 0., max(f1x, f2x));

	return colour;
}