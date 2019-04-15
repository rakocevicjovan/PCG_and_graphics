cbuffer LightBuffer : register(b0)
{
	float3 alc;
	float ali;
	float3 dlc;
	float dli;
	float3 slc;
	float sli;
	float4 lightDir;
	float4 eyePos;
	float elapsed;
	float3 padding;
	float4x4 lightView;
};

Texture2D worley : register(t0);
SamplerState CloudSampler : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : WPOS;
    float4 msPos : MSPOS;
};


///Constants
static const float3 BG_COL = float3(0.7f, 0.6f, 0.4f);
static const int2 iResolution = int2(1600, 900);
static const float GAIN = 0.707f;
static const float LACUNARITY = 2.5789f;

static const float STEPS = 40.0;
static const float STEP_SIZE = 1.f / STEPS;

static const float3 ISOLIGHTTOP = float3(0.6f, 0.7f, 0.9f);
static const float3 ISOLIGHTBOT = float3(0.1f, 0.5f, 0.5f);
///Constants done //r(d) = e ^ ( -O`(t) * d);



float3 mod289(float3 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 mod289(float4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 permute(float4 x)
{
    return mod289(((x * 34.0) + 1.0) * x);
}

float4 taylorInvSqrt(float4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(float3 v)
{
    const float2 C = float2(1.0 / 6.0, 1.0 / 3.0);
    const float4 D = float4(0.0, 0.5, 1.0, 2.0);

	// First corner
    float3 i = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

	// Other corners
    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
    float3 x1 = x0 - i1 + C.xxx;
    float3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    float3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

	 // Permutations
    i = mod289(i);
    float4 p = permute(permute(permute(i.z + float4(0.0, i1.z, i2.z, 1.0)) + i.y + float4(0.0, i1.y, i2.y, 1.0)) + i.x + float4(0.0, i1.x, i2.x, 1.0));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    float3 ns = n_ * D.wyz - D.xzx;

    float4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,7*7)

    float4 x_ = floor(j * ns.z);
    float4 y_ = floor(j - 7.0 * x_); // mod(j,N)

    float4 x = x_ * ns.x + ns.yyyy;
    float4 y = y_ * ns.x + ns.yyyy;
    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

	//float4 s0 = float4(lessThan(b0,0.0))*2.0 - 1.0;
	//float4 s1 = float4(lessThan(b1,0.0))*2.0 - 1.0;
    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, float4(0., 0., 0., 0.));

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 p0 = float3(a0.xy, h.x);
    float3 p1 = float3(a0.zw, h.y);
    float3 p2 = float3(a1.xy, h.z);
    float3 p3 = float3(a1.zw, h.w);

	//Normalise gradients
    float4 norm = taylorInvSqrt(float4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

	// Mix final noise value
    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, float4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}



//fractal brownian motion across the texture
float map5(in float3 p)
{

	float3 q = p - float3(0.3, 0.1, 0.2) * elapsed;
	float f = 0.50000f  * snoise(q);

	q = q * LACUNARITY;
    f += 0.25000f * snoise(q);

	q = q * LACUNARITY;
    f += 0.12500f * snoise(q);

	q = q * LACUNARITY;
    f += 0.06250f * snoise(q);

	q = q * LACUNARITY;
    f += 0.03125f * snoise(q);

	return clamp(f, 0.0f, 1.0f);
}


float4 integrate(in float4 sum, in float dif, in float den, in float t)
{
	// lighting
	float3 lin = dlc * 1.4f + slc * dif;
	float4 col = float4(lerp(slc, alc, den), den);
	col.xyz *= lin;
	col.xyz = lerp(col.xyz, BG_COL, 1.0 - exp(-0.003f * t * t));
	// front to back blending    
	col.a *= 0.4f;
	col.rgb *= col.a;
	return sum + col * (1.0f - sum.a);
}



void march(in float3 rayOrigin, in float3 rayDir, inout float4 sum)
{
	float density = 0.0;
	float t = 0.0;

	for (int i = 0; i < STEPS; i++)
	{
		float3 pos = rayOrigin + rayDir * t;

		if (sum.a > 0.99f)
			break;

		density = map5(pos);

		if (density > 0.01f)
		{
			//dif is the difference between current density and density of the area closer to the light, clamped 0 to 1
			float dif = clamp((density - map5(pos - 0.3f * lightDir.xyz)), 0.0f, 1.0f);
			sum = integrate(sum, dif, density, t);
		}

		t += STEP_SIZE;
	}
}



float4 raymarch(in float3 rayOrigin, in float3 rayDir)
{
	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);

	march(rayOrigin, rayDir, sum);

	return clamp(sum, 0.0f, 1.0f);
}



float4 render(in float3 rayOrigin, in float3 rayDir)
{

	//background sky, this is quite easy to understand as it's just blending the background light with a specular factor
	//float moonbeams = clamp(dot(-lightDir.xyz, rayDir), 0.0f, 1.0f);
	//float3 background = BG_COL + slc * pow(moonbeams, 16.0f);

	//clouds
	float4 cloudColour = raymarch(rayOrigin, rayDir);
	return cloudColour;
	//float bgAlpha = (1.0f - cloudColour.a);

	//background = background * bgAlpha + cloudColour.rgb;	//background colour is "overwritten" by cloud colour

	//return float4(background, pow(cloudColour.a, 5));
}



float4 strifeFragment(PixelInputType input) : SV_TARGET
{
	float4 colour;
	float3 rayDir = normalize(input.worldPos.xyz - eyePos.xyz);
		
	colour = render(input.worldPos.xyz, rayDir);

	return colour;
}