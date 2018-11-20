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

	float3 rg = white.Sample(CloudSampler, (uv + 0.5f) / 256.0f);
	//float3 rg = white.Load(int3(uv.x, uv.y, 0.0f)).x;

	return -1.0 + 2.0 * lerp(rg.r, rg.g, f.b);	//return 1 - noise value
}

float sampleDensityOfMedium(in float3 pos) {

	//octave 1
	float3 UVW = pos * 0.01f; // Let’s start with some low frequency
	float Amplitude = 1.0f;
	float V = Amplitude * noise(UVW);

	//octave 2
	Amplitude *= AMPLITUDE_FACTOR;
	UVW *= FREQUENCY_FACTOR;
	V += Amplitude * noise(UVW);

	//octave 3
	Amplitude *= AMPLITUDE_FACTOR;
	UVW *= FREQUENCY_FACTOR;
	V += Amplitude * noise(UVW);

	//octave 4
	Amplitude *= AMPLITUDE_FACTOR;
	UVW *= FREQUENCY_FACTOR;
	V += Amplitude * noise(UVW);

	//octave 5
	Amplitude *= AMPLITUDE_FACTOR;
	UVW *= FREQUENCY_FACTOR;
	V += Amplitude * noise(UVW);

	return clamp(V, 0, 1); // Include factor and bias instead of just V to help getting a nice result… -> DensityFactor * V + DensityBias
}

float3 computeMoonColour(in float3 pos) {

	float3 ShadowMapPosition = mul(pos, (float3x3)lightView); // Transform from world to shadow map space
	float2 UV = ShadowMapPosition.xy; // Our shadow map texture coordinates in [0,1]
	float Z = ShadowMapPosition.z; // Our depth in the shadow volume as seen from the light
	float Extinction = noise(ShadowMapPosition); // Samples the shadow map and returns extinction in [0,1]	GetShadowExtinction(UV, Z);
	return Extinction * dlc; // Attenuate moon color by extinction through the volume
}

float ei(in float z) {
	return 0.577215f + log(1e-4 + abs(z)) + z * (1.0 + z * (0.25 + z * ((1.0 / 18.0) + z * ((1.0 / 96.0) + z * (1.0 / 600.0))))); // For x!=0
}

float3 computeAmbientColour(in float3 pos, in float extK) {

	float distToTop = 500.0f - pos.y; // Height to the top of the volume
	float a = -extK * distToTop;
	float3 IsotropicScatteringTop = ISOLIGHTTOP * max(0.0, exp(a) - a * ei(a));

	float distToBottom = pos.y - 300.0f; // Height to the bottom of the volume
	a = -extK * distToBottom;
	float3 IsotropicScatteringBottom = ISOLIGHTBOT * max(0.0f, exp(a) - a * ei(a));

	return IsotropicScatteringTop + IsotropicScatteringBottom;
}


float4 myMarch(in float3 rayOrigin, in float3 rayDir, in int numSteps) {

	//set up
	float extinction = 1.0f;						//no density anywhere at first so no extinction either...
	float3 scattering = float3(0.0f, 0.0f, 0.0f);	//no light scattering at first
	float stepSize = 1.0f / numSteps;				//randomly chose 2 to be max distance idk... should play with it
	float3 pos = rayOrigin;

	//marching code
	for (int i = 0; i < numSteps; ++i) {

		float timeK = 10.f;
		float3 offset = float3(elapsed, elapsed, elapsed) * timeK;
		float density = sampleDensityOfMedium(pos + offset);	// Sample perlin/worley/white noise and clamp the result between 0 and 1
		float scatteringK = 0.1f * density;			//scatteringFactor
		float extinctionK =  0.01f * density;			//extinctionFactor

		extinction *= exp(-extinctionK * stepSize);	//pass step sign - faster to calculate once and keep passing it!

		//compute light added by scattering
		float3 moonCol = computeMoonColour(pos);	//moonlight color arriving at the position
		float3 ambientCol = dlc; //computeAmbientColour(pos, extinctionK);
		float3 stepScattering = scatteringK * stepSize * ( 5.f* dlc + 5.f * alc);	//how to implement phase function - phaseMoon, phaseAmbient
		scattering += extinction * stepScattering; //add scattering but attenuate it... so the more extinct the light is at that point the less we add

		pos += stepSize * rayDir;
	}

	return float4(scattering, extinction);
}






//fractal brownian motion across the texture
float map5(in float3 p) {

	float3 q = p - float3(0.3, 0.1, 0.2) * sin(elapsed * 0.314159f);
	float f = 0.50000f  * noise(q);

	q = q * 2.02f;
	f += 0.25000f * noise(q);

	q = q * 2.03f;
	f += 0.12500f * noise(q);

	q = q * 2.01f;
	f += 0.06250f * noise(q);

	q = q * 2.02f;
	f += 0.03125f * noise(q);

	return clamp(1.5f - p.y - 2.0f + 1.75f * f, 0.0f, 1.0f);
}


float4 integrate(in float4 sum, in float dif, in float den, in float t) {
	// lighting
	float3 lin = float3(0.65f, 0.7f, 0.75f) * 1.4f + float3(1.0f, 0.6f, 0.3f) * dif;
	float4 col = float4(lerp(float3(1.0f, 0.95f, 0.8f), float3(0.25f, 0.3f, 0.35f), den), den);
	col.xyz *= lin;
	col.xyz = lerp(col.xyz, BG_COL, 1.0 - exp(-0.003f * t * t));
	// front to back blending    
	col.a *= 0.4f;
	col.rgb *= col.a;
	return sum + col * (1.0f - sum.a);
}


void march(in int steps, in float3 rayOrigin, in float3 rayDir, in float initialT, inout float4 sum) {

	float density;
	float t = initialT;

	for (int i = 0; i < steps; i++) {
		float3 pos = t * rayDir;
		if (pos.y < -3.0f || pos.y > 2.0f || sum.a > 0.99f) break;
		density = map5(pos);

		if (density > 0.01f) {
			//dif is the difference between current density and density of the area closer to the light, clamped 0 to 1
			float dif = clamp((density - map5(pos - 0.3f * lightDir.xyz)) / 0.6f, 0.0f, 1.0f);
			sum = integrate(sum, dif, density, t);
		}
		t += max(0.05f, 0.02f * t);
	}

	//if (density < 0.33f) discard;
}


float4 raymarch(in float3 rayOrigin, in float3 rayDir) {

	//initialize the sum of colours we will get in the end
	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);

	int2 xy = rayOrigin.xz;

	float t = perlin.Load(int3(xy.x, xy.y, 0.0f)).x;	//x because it's grayscale so any value would do...

	march(100, rayOrigin, rayDir, t, sum);

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


	return float4(background, 1.0f - bgAlpha * bgAlpha * bgAlpha);
}



float4 strifeFragment(PixelInputType input) : SV_TARGET{

	float4 colour;
	float3 rayDir = normalize(input.worldPos.xyz - eyePos.xyz);

	//float3 cloudCenter = float3(0.0f, 500.0f, 500.0f);
	//float3 relPos = input.worldPos.xyz - cloudCenter;
		
	colour = render(input.worldPos.xyz, rayDir);	// relPos rayDir
	//colour =  saturate(myMarch(input.worldPos.xyz, rayDir, 64));
	
	//colour.a = 1.0f;// length(colour.xyz);

	return colour;
}