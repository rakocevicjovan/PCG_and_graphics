cbuffer VariableBuffer : register(b0)
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
	float4 msPos : MSPOS;
	float4 wPos : WPOS;
};

Texture2D shaderTexture;
SamplerState SampleType;


float remap(float value, float min1, float max1, float min2, float max2)
{
	return min2 + ((value - min1) / (max1 - min1)) * (max2 - min2);
}

float3 mod289(float3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 mod289(float4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 permute(float4 x) {
	return mod289(((x*34.0) + 1.0)*x);
}

float4 taylorInvSqrt(float4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(float3 v)
{
	const float2  C = float2(1.0 / 6.0, 1.0 / 3.0);
	const float4  D = float4(0.0, 0.5, 1.0, 2.0);

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
	float3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

	 // Permutations
	i = mod289(i);
	float4 p = permute(permute(permute(i.z + float4(0.0, i1.z, i2.z, 1.0)) + i.y + float4(0.0, i1.y, i2.y, 1.0)) + i.x + float4(0.0, i1.x, i2.x, 1.0));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	float3  ns = n_ * D.wyz - D.xzx;

	float4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	float4 x_ = floor(j * ns.z);
	float4 y_ = floor(j - 7.0 * x_);    // mod(j,N)

	float4 x = x_ * ns.x + ns.yyyy;
	float4 y = y_ * ns.x + ns.yyyy;
	float4 h = 1.0 - abs(x) - abs(y);

	float4 b0 = float4(x.xy, y.xy);
	float4 b1 = float4(x.zw, y.zw);

	//float4 s0 = float4(lessThan(b0,0.0))*2.0 - 1.0;
	//float4 s1 = float4(lessThan(b1,0.0))*2.0 - 1.0;
	float4 s0 = floor(b0)*2.0 + 1.0;
	float4 s1 = floor(b1)*2.0 + 1.0;
	float4 sh = -step(h, float4(0., 0., 0., 0.));

	float4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
	float4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

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
	return 42.0 * dot(m*m, float4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}



float randomizer(in float2 seed)
{
	seed = frac(seed * float2(5.3983, 5.4427));
	seed += dot(seed.yx, seed.xy + float2(21.5351, 14.3137));
	return frac(seed.x * seed.y * 95.4337);
}

//Helpers
static const float PI = 3.141592f;
static const float INTENSITY = 1.61803f * PI;
static const float3 PARTICLE_OFFSET = float3(-.5, .707, -.33);

//FBM settings
static const int NUM_OCTAVES = 7;
static const float LACUNARITY = 2.13795;
static const float GAIN = .497531;


/*not used *//*
float calcEmbers(inout float3 diffuse, float3 norm, float time, float2 coord)
{
	diffuse *= (0.5 + 0.5 * norm.x) * 2.5 + float3(1.0, 0.35, 0.04) * 0.02;	//stronger colour when norm points to the right, weaker when to the left

	//a bit of variation in the intensity using the time, really subtle though, and using the fbm with offset
	float embers = smoothstep(0.77 + 0.01 * (sin(elapsed * 20.0) + sin(elapsed)), 1.0, fbm(coord*10.0 + float2(cos(coord.y * 0.8 + time * 0.7) * 10.0, time * 4.0)));

	embers += smoothstep(0.77 + sin(time*22.0)*0.01 + sin(time*1.2)*0.01, 1.0, fbm(float2(100.0) + coord * 8.0 + float2(time*8.0 + cos(coord.y*0.3 + time * 0.3)*10.0, time*7.0)));
	return embers;
}*/


float fbm(in float3 pos)
{
	float v = 0.0;

	float amplitude = 1.f;
	float frequency = 1.f;

	for (int i = 0; i < NUM_OCTAVES; ++i)
	{
		v += snoise(frequency * pos) * amplitude;
		frequency *= LACUNARITY;
		amplitude *= GAIN;
	}
	return v;
}



float turbulentFBM(float3 x)
{
	float sum = 0.0f;

	float frequency = 1.0f;
	float amplitude =	 1.0f;

	for (int i = 0; i < NUM_OCTAVES; ++i)
	{
		float r = snoise(frequency * x) * amplitude;
		r = r < 0 ? -r : r;
		sum += r;
		frequency *= LACUNARITY;
		amplitude *= GAIN;
	}

	return sum;
}


float noiseStack(float3 pos) {

	float noise = snoise(float3(pos));
	float off = 1.0;

	pos *= LACUNARITY;
	off *= GAIN;
	noise = (1.0 - off)* noise + off * snoise(float3(pos));

	pos *= LACUNARITY;
	off *= GAIN;
	noise = (1.0 - off) * noise + off * snoise(float3(pos));

	pos *= LACUNARITY;
	off *= GAIN;
	noise = (1.0 - off) * noise + off * snoise(float3(pos));

	return (1.0 + noise) / 2.0;
}



float calcParticle(in float3 curPos)
{
	float3 between = PARTICLE_OFFSET - curPos * sin(elapsed);
	return smoothstep(.02f, 0., dot(between, between));
}


//Raymarch settings
static const int NUM_STEPS = 7;
static const float STEP_SIZE = 2.f / (float)NUM_STEPS;

float4 raymarch(in float3 rayOrigin, in float3 rayDir)
{
	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float particle;

	float flame = 0.f;
	float t = 0.f;
	float3 noiseDir = float3(0., .66, .33);
	
	for (int i = 0; i < NUM_STEPS; ++i)
	{
		if (flame > 95.f)	break;

		float3 curPos = rayOrigin + t * rayDir;

		//float3 curPosAdj = float3(abs(curPos.x), curPos.y, curPos.z);
		float3 rotatedPos = mul(curPos, rotationMatrix);
		float3 noisePos = rotatedPos - noiseDir * elapsed;
		
		float dist = length(curPos) - .33f * snoise(noisePos);	//get dist to middle of sphere
		float mask = smoothstep(.707, 0., dist);
		
		flame += 
				STEP_SIZE * 
				mask * 
				turbulentFBM(noisePos * 2.1f) *
				INTENSITY;

		t += STEP_SIZE;
	}

	sum = smoothstep(float4(0., 0., 0., 0.), float4(1.2, 1.2, 1.2, 1.), float4(flame, flame * flame, .7f * flame * flame, flame));
	
	return sum;
}



float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float x = input.msPos.x;
	float y = input.msPos.y;
	float z = input.msPos.z;
	float3 xyz = float3(x, y, z);

	float3 viewdir = normalize(input.wPos.xyz - eyePos.xyz); //ray direction

	float4 colour = raymarch(xyz, viewdir);	//ray origin is the first pixel hit
	
	//colour.rgb = pow(colour.rgb, float3(1.0f / 3.2f, 1.0f / 3.2f, 1.0f / 3.2f));

	return colour;
}



//curPos.xy = mul(curPos.xz, rotMat);
//curPos = normalize(curPos);	//renormalize
//curPos.y -= elapsed;

//float thokkMakeBigFire =	(mask * snoise(curPos))			*	STEP_SIZE * 25.f;
//float disturbance =			(mask * noiseStack(curPos))	*	STEP_SIZE * 25.f;

//float final = max(thokkMakeBigFire, disturbance);	//, .5f