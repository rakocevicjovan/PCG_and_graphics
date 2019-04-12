cbuffer VariableBuffer : register(b1)
{
	float elapsed;
	float3 padding;
};



cbuffer ViewDirBuffer : register(b1)
{
	float4x4 rotationMatrix;
	float4 eyePos;
};



struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 msPos : WATDIS;
	float4 wPos : WPOS;
};

static const float invSqrt2 = 0.707107;	//same as sin 45 and cos45
static const float cossin45 = 0.707107;
static const float2x2 rotMat1 = { cossin45, cossin45, -cossin45, cossin45 };
static const float2x2 rotMat2 = { -cossin45, cossin45, -cossin45, cossin45 };

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	//float4 colour;

	//float2 f1x = invSqrt2 * mul(input.msPos.xz, rotMat1);
	//float2 f2x = invSqrt2 * mul(input.msPos.xz, rotMat2) + float2(1.0, 0.0);

	return float4(1.f, 1.f, 1.f, 1.f);
}


/*
static const float invSqrt2 = 0.707107;	//same as sin 45 and cos45
static const float cossin45 = 0.707107;
static const float2x2 rotMat1 = { cossin45, cossin45, -cossin45, cossin45 };
static const float2x2 rotMat2 = { -cossin45, cossin45, -cossin45, cossin45 };

//float2 f1x = invSqrt2 * mul(input.msPos.xz, rotMat1);
//float2 f2x = invSqrt2 * mul(input.msPos.xz, rotMat2) + float2(1.0, 0.0);
*/