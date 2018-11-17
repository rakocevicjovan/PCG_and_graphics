cbuffer LightBuffer {
	float3 alc;
	float ali;
	float3 dlc;
	float dli;
	float3 slc;
	float sli;
	float4 lightPosition;
	float4 eyePos;
};


struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
};

Texture2D noiseTexture;
SamplerState CloudSampler;


float noise(in float3 x){

	float3 p = floor(x);
	float3 f = frac(x);
	f = f * f*(3.0 - 2.0*f);


	//makes it "3d"... but how? It adds a second vector that's based on "depth" aka z value times the size of the image to sample with offset 
	float2 uv = (p.xy + float2(37.0f, 17.0f) * p.z) + f.xy;

	//that's not how it works in directX...	
	///iChannel0 is a noise texture (Noiz.png) and 256 is the length of the width and height of the texture (it's a square)
	float2 rg = noiseTexture.SampleLevel(CloudSampler, (uv + 0.5f) / 256.0f, 0.f).yx;
	
	/*	//wat dis code do? it was defined out
	// #if 1 #else	#endif 
	
	int3 q = int3(p);
	int2 uv = q.xy + int2(37, 17)*q.z;

	float2 rg = mix(mix(texelFetch(iChannel0, (uv) & 255, 0),
		texelFetch(iChannel0, (uv + int2(1, 0)) & 255, 0), f.x),
		mix(texelFetch(iChannel0, (uv + int2(0, 1)) & 255, 0),
			texelFetch(iChannel0, (uv + int2(1, 1)) & 255, 0), f.x), f.y).yx;
	*/
   

	return -1.0 + 2.0 * lerp(rg.x, rg.y, f.z);	//return 1 - noise value
}

//from what to what?	what are p, q and f?
float map5(in float3 p, float dTime){

	float3 q = p - float3(0.0, 0.1, 1.0) * dTime;	//pass dTime here!
	float f;										//texture coordinates
	f = 0.50000f  * noise(q); 
	q = q * 2.02f;
	f += 0.25000f * noise(q); 
	q = q * 2.03f;
	f += 0.12500f * noise(q); 
	q = q * 2.01f;
	f += 0.06250f * noise(q); 
	q = q * 2.02f;
	f += 0.03125f * noise(q);
	return clamp(1.5f - p.y - 2.0f + 1.75f * f, 0.0f, 1.0f);	//map to 0-1 range
}


float map4(in float3 p, float dTime){

	float3 q = p - float3(0.0f, 0.1f, 1.0f) * dTime;
	float f;
	f  = 0.5000f * noise(q); 
	q = q * 2.02f;
	f += 0.2500f * noise(q); 
	q = q * 2.03f;
	f += 0.1250f * noise(q); 
	q = q * 2.01f;
	f += 0.0625f * noise(q);

	return clamp(1.5 - p.y - 2.0 + 1.75*f, 0.0, 1.0);	//again map to 0-1 range
}


float map3(in float3 p, float dTime){

	float3 q = p - float3(0.0f, 0.1f, 1.0f) * dTime;
	float f;
	f = 0.50000*noise(q); 
	q = q * 2.02;
	f += 0.25000*noise(q); 
	q = q * 2.03;
	f += 0.12500*noise(q);
	return clamp(1.5f - p.y - 2.0f + 1.75f * f, 0.0, 1.0);
}


float map2(in float3 p, float dTime){

	float3 q = p - float3(0.0f, 0.1f, 1.0f) * dTime;
	float f;
	f  = 0.50000f * noise(q); 
	q = q * 2.02f;
	f += 0.25000f * noise(q);;
	return clamp(1.5f - p.y - 2.0f + 1.75f * f, 0.0f, 1.0f);
}

float4 integrate(in float4 sum, in float dif, in float den, in float3 bgcol, in float t){
	// lighting
	float3 lin = float3(0.65f, 0.7f, 0.75f) * 1.4f + float3(1.0f, 0.6f, 0.3f) * dif;
	float4 col = float4(mix(float3(1.0f, 0.95f, 0.8f), float3(0.25f, 0.3f, 0.35f), den), den);
	col.xyz *= lin;
	col.xyz = mix(col.xyz, bgcol, 1.0 - exp(-0.003*t*t));
	// front to back blending    
	col.a *= 0.4f;
	col.rgb *= col.a;
	return sum + col * (1.0f - sum.a);
}


//make sense of this
/*
#define MARCH(STEPS,MAPLOD) for(int i=0; i<STEPS; i++) { vec3  pos = ro + t*rd; if( pos.y<-3.0 || pos.y>2.0 || sum.a > 0.99 ) break; 
float den = MAPLOD(pos); if (den > 0.01) { float dif = clamp((den - MAPLOD(pos + 0.3*sundir)) / 0.6, 0.0, 1.0); sum = integrate(sum, dif, den, bgcol, t); }
t += max(0.05, 0.02*t); }
*/

float4 march(in int steps, in float3 ro, in float3 rd, in float t, inout sum, float dTime) {
	for (int i = 0; i < steps; i++) {
		float3 pos = ro + t * rd;
		if (pos.y < -3.0f || pos.y > 2.0f || sum.a > 0.99f) break;
		float den = map5(pos, dTime);

		if (den > 0.01f) {
			float dif = clamp((den - map5(pos + 0.3f * lightDir)) / 0.6f, 0.0f, 1.0f);
			sum = integrate (sum, dif, den, bgcol, t);
		}
		t += max(0.05f, 0.02f * t);
	}
}

float4 raymarch(in float3 ro, in float3 rd, in float3 bgcol, in int2 px, float dTime){
	
	float4 sum = float4(0.0);

	//0.05*texelFetch( iChannel0, px&255, 0 ).x;
	float t = 0.05f * noiseTexture.Load(px&255).x;	//x because it's grayscale so any value would do...
	
	/*MARCH(30, map5);
	MARCH(30, map4);
	MARCH(30, map3);
	MARCH(30, map2);*/

	march(30, ro, rd, t, sum, dTime);
	
	return clamp(sum, 0.0f, 1.0f);
}



float4 render(in float3 ro, in float3 rd, in int2 px, float dTime){

	// background sky     
	float sun = clamp(dot(sundir, rd), 0.0f, 1.0f);
	float3 col = float3(0.6f, 0.71f, 0.75f) - rd.y * 0.2f * float3(1.0f, 0.5f, 1.0f) + 0.15f * 0.5f;
	col += 0.2f * float3(1.0f, .6f, 0.1f) * pow(sun, 8.0f);

	// clouds    
	float4 res = raymarch(ro, rd, col, px, dTime);
	col = col * (1.0f - res.w) + res.xyz;

	// sun glare    
	col += 0.2 * float3(1.0f, 0.4f, 0.2f) * pow(sun, 3.0f);

	return float4(col, 1.0f);
}


//...in between these comment lines
float4 strifeFragment(PixelInputType input){

	float4 colour;
	float2 p = (-iResolution.xy + 2.0*fragCoord.xy) / iResolution.y;

	// ray
	float3 rd = ca * normalize(float3(p.xy, 1.5f));

	colour = render(ro, rd, int2(fragCoord - 0.5f), dTime);
	return colour;
}


/*
//Got my own camera, don't need this.

float3x3 setCamera(in float3 ro, in float3 ta, float cr){

	float3 cw = normalize(ta - ro);
	float3 cp = float3(sin(cr), cos(cr), 0.0);
	float3 cu = normalize(cross(cw, cp));
	float3 cv = normalize(cross(cu, cw));
	return float3x3(cu, cv, cw);
}

// from main
	float3 ro = 4.0*normalize(float3(sin(3.0f * m.x), 0.4f * m.y, cos(3.0f * m.x)));
	float3 ta = float3(0.0f, -1.0f, 0.0f);
	float3 ca = setCamera(ro, ta, 0.0f);
*/


/*//for VR I guess? Don't know! Possibly don't care this is tough enough as it is...
void mainVR(out float4 fragColor, in float2 fragCoord, in float3 fragRayOri, in float3 fragRayDir){
	fragColor = render(fragRayOri, fragRayDir, int2(fragCoord - 0.5));
}
*/