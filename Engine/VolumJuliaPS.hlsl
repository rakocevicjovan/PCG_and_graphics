cbuffer LightBuffer : register(b0)
{
	float3 alc;
	float ali;
	float3 dlc;
	float dli;
	float3 slc;
	float sli;
	float4 lightPosition;
};

cbuffer VariableBuffer : register(b1)
{
	float elapsed;
	float3 padding;
};

cbuffer ViewDirBuffer : register(b2)
{
	float4 eyePos;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 msPos : MSPOS;
	float4 wPos : WPOS;
};


///	Julia set in 2D
static const float JULIA_STEPS = 256.f;
static const float JULIA_STEP_SIZE = 1.f / JULIA_STEPS;
static const float2 JULIA_C = float2(-0.70176, -0.3842);

float2 cmul(float2 c1, float2 c2)
{
	return float2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

float4 julia2D(float4 pos)
{
	float4 colour = { 0.f, 0.f, 0.f, 0.f };

	float den = pos.z - elapsed * .033f;
	float co = cos(den);
	float si = sin(den);
	matrix<float, 2, 2> rotMat = { co, -si, si, co };

	float den2 = den * 10.f;
	co = cos(den);
	si = sin(den);
	matrix<float, 2, 2> rotMat2 = { co, -si, si, co };

	float2 tZ = abs(pos.xy) * 3.33f;
	tZ = mul(tZ, rotMat2);


	for (int s = 0; s < JULIA_STEPS; ++s)
	{
		tZ = mul(tZ, rotMat);
		tZ = cmul(tZ, tZ);
		tZ += JULIA_C;
		if (tZ.x * tZ.x + tZ.y * tZ.y > 4.f)
		{
			float r = s * JULIA_STEP_SIZE;
			float g = 0.f;
			float b = s * JULIA_STEP_SIZE * 1.6f;
			float a = s * JULIA_STEP_SIZE * 10.f; //1.f;

			return float4(r, g, b, a);
		}

	}

	return colour;
}




/*
static const float RM_STEPS = 64.f;
static const float RM_STEP_SIZE = 1.f / RM_STEPS;
static const float4 c = { -0.213,-0.0410,-0.563,-0.560 };
static const float J3D_STEPS = 14.;
static const float J3D_STEP_SIZE = 1. / 14.;
static const float J3D_MIN = 0.003f;
static const float EPSILON = 1e-5;

static const float k = .55f * 3.f;
static const float tw = 1.33f;
static const float4 quat = float4(
	sin(elapsed * tw) * 0.451 * k,
	cos(elapsed * tw) * 0.435 * k,
	sin(elapsed * tw) * 0.396 * k,
	cos(elapsed * tw) * 0.425 * k);

float4 qmul(float4 a, float4 b)
{
	return float4
		(
			a.w * b.xyz + b.w * a.xyz + cross(a.xyz, b.xyz),
			a.w * b.w - dot(a.xyz, b.xyz)
		);
}

float julia3D(float3 pos, float4 quat)
{
	float4 newZ;
	float4 z = float4(pos, 0.0);
	float zSquared = dot(pos, pos);
	float mdSquared = 1.0;

	for (int i = 0; i < J3D_STEPS; i++)
	{
		mdSquared *= 4.0 * zSquared;
		newZ.x = z.x*z.x - dot(z.yzw, z.yzw);
		newZ.y = 2.0*(z.x*z.y + z.w*z.z);
		newZ.z = 2.0*(z.x*z.z + z.w*z.y);
		newZ.w = 2.0*(z.x*z.w - z.y*z.z);
		z = newZ + quat;
		zSquared = dot(z, z);
		if (zSquared > 4.0) break;
	}
	return 0.25 * sqrt(zSquared / mdSquared) * log(zSquared);
}



float3 calcNormal(float3 p) {
	float3 n;
	n.x = julia3D(float3(p.x + EPSILON, p.y, p.z), quat);
	n.y = julia3D(float3(p.x, p.y + EPSILON, p.z), quat);
	n.z = julia3D(float3(p.x, p.y, p.z + EPSILON), quat);
	return normalize(n - julia3D(p, quat));
}
*/


float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	/*
	float3 viewDir = normalize(input.wPos.xyz - eyePos.xyz);
	float3 increment = RM_STEP_SIZE * viewDir;

	float r = 0.f;
	float3 curPos = input.msPos.xyz;
	int i = 0;
	
	for (i = 0; i < RM_STEPS; ++i)
	{
		float distToJuliaSet = julia3D(curPos, quat);
		r += distToJuliaSet * RM_STEP_SIZE * .1f;

		if (r < -1.0f)	//distToJuliaSet < J3D_MIN ||
			break;

		curPos += increment;
	}

	r /= (float)(i * 0.05f);

	float3 n = calcNormal(curPos);

	return float4(-r, -r, -r, -r);
	*/

	
	return julia2D(input.msPos * 2.f);
}


