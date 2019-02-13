cbuffer LightBuffer {
	float3 alc;
	float ali;
	float3 dlc;
	float dli;
	float3 slc;
	float sli;
	float4 lightDir;
	float4 eyePos;
	float4 viewDir;
	float elapsed;
	float3 padding;
	float4x4 lightView;
};

Texture2D white : register(t0);
Texture2D perlin : register(t1);
Texture2D worley : register(t2);

SamplerState CloudSampler : register(s0);

struct PixelInputType {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
};


///Constants
static const float3 BG_COL = float3(0.1f, 0.05f, 0.15f);
static const int2 iResolution = int2(1600, 900);
static const float AMPLITUDE_FACTOR = 0.707f; // Decrease amplitude by sqrt(2)/2
static const float FREQUENCY_FACTOR = 2.5789f; // Increase frequency by some factor each new octave
//light coming from the moon and light reflected from below
static const float3 ISOLIGHTTOP = float3(0.6f, 0.7f, 0.9f);
static const float3 ISOLIGHTBOT = float3(0.1f, 0.5f, 0.5f);
///Constants done //r(d) = e ^ ( -O`(t) * d);


float noise(in float3 x) {

	float3 p = floor(x);
	float3 f = frac(x);
	f = f * f * (3.0 - 2.0*f);

	float2 uv = (p.xy + float2(370.0f, 170.0f) * p.z) + f.xy;

	float3 rg = white.SampleLevel(CloudSampler, (uv + 0.5f) / 256.0f, 0);
	//float3 rg = white.Load(int3(uv.x, uv.y, 0.0f)).x;

	return -1.0 + 2.0 * lerp(rg.r, rg.g, f.b);	//return 1 - noise value
}






//fractal brownian motion across the texture
float map5(in float3 p) {

	float3 q = p - float3(0.3, 0.1, 0.2) * elapsed * 0.314159f;
	float f = 0.50000f  * noise(q);

	q = q * FREQUENCY_FACTOR;
	f += 0.25000f * noise(q);

	q = q * FREQUENCY_FACTOR;
	f += 0.12500f * noise(q);

	q = q * FREQUENCY_FACTOR;
	f += 0.06250f * noise(q);

	q = q * FREQUENCY_FACTOR;
	f += 0.03125f * noise(q);

	return clamp(f, 0.0f, 1.0f);
}


float4 integrate(in float4 sum, in float dif, in float den, in float t) {
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


void march(in int steps, in float3 rayOrigin, in float3 rayDir, inout float4 sum) {

	float density;
	float t = 0.0f;
	float stepSize = 1.0f / steps;

	for (int i = 0; i < steps; i++) {
		float3 pos = t * rayDir;
		//if (pos.y < -3.0f || pos.y > 2.0f || sum.a > 0.99f) break;
		if (sum.a > 0.99f) break;
		density = map5(pos % 256);

		if (density > 0.01f) {
			//dif is the difference between current density and density of the area closer to the light, clamped 0 to 1
			float dif = clamp((density - map5(pos - 0.3f * lightDir.xyz)) / 0.6f, 0.0f, 1.0f);
			sum = integrate(sum, dif, density, t);
		}
		t += stepSize; //max(0.05f, 0.02f * t);
	}

	//if (density < 0.33f) discard;
}


float4 raymarch(in float3 rayOrigin, in float3 rayDir) {

	//initialize the sum of colours we will get in the end
	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);

	march(50, rayOrigin, rayDir, sum);

	return clamp(sum, 0.0f, 1.0f);
}



float4 render(in float3 rayOrigin, in float3 rayDir) {

	//background sky, this is quite easy to understand as it's just blending the background light with a specular factor
	float moonbeams = clamp(dot(-lightDir.xyz, rayDir), 0.0f, 1.0f);
	float3 background = BG_COL + slc * pow(moonbeams, 16.0f);

	//clouds
	float4 cloudColour = raymarch(rayOrigin, rayDir);

	float bgAlpha = (1.0f - cloudColour.a);

	background = background * bgAlpha + cloudColour.rgb;	//background colour is "overwritten" by cloud colour


	return float4(background, pow(cloudColour.a, 5));
}



float4 strifeFragment(PixelInputType input) : SV_TARGET{

	float4 colour;
	float3 rayDir = normalize(input.worldPos.xyz - eyePos.xyz);

	//float3 cloudCenter = float3(0.0f, 500.0f, 500.0f);
	//float3 relPos = input.worldPos.xyz - cloudCenter;
		
	colour = render(input.worldPos.xyz, rayDir);	// relPos rayDir
	
	//colour.a = 1.0f;// length(colour.xyz);

	return colour;
}