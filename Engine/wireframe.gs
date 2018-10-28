/*

#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec4 wPos[3];

noperspective out vec3 distance;

void main(){

	vec2 viewportSize = vec2(1920, 1080);
	//float viewportSize = 1920*1080;
	
	vec3 ndc = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
	vec2 viewportCoord = ndc.xy * 0.5 + 0.5;	//going from ndc range [-1, 1] to 
	vec2 p0 = viewportCoord * viewportSize;

	ndc = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
	viewportCoord = ndc.xy * 0.5 + 0.5;
	vec2 p1 = viewportCoord * viewportSize;

	ndc = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;
	viewportCoord = ndc.xy * 0.5 + 0.5;
	vec2 p2 = viewportCoord * viewportSize;


	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);

	float alpha = acos( (b*b + c*c - a*a) / (2.0*b*c) );
	float beta = acos( (a*a + c*c - b*b) / (2.0*a*c) );

	float ha = abs( c * sin( beta ) );
	float hb = abs( c * sin( alpha ) );
	float hc = abs( b * sin( alpha ) );
	
	//for (int i = 0; i < gl_in.length(); i++){}	//pain in the ass to assign heights in a loop...

	distance = vec3(ha, 0, 0);
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	distance = vec3(0, hb, 0);
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	distance = vec3(0, 0, hc);
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
     
	EndPrimitive();
}

*/

///////////////////////

struct GeomInputType{
    float4 position : SV_POSITION;
	float4 worldPos : WPOS;
};

struct Pyxis{
    float4 position : SV_POSITION;
	noperspective float3 distance : DIST;
};


[maxvertexcount(3)]
void WFGS(triangle GeomInputType myVertInp[3], inout TriangleStream<Pyxis> OutputStream  ){
	
	Pyxis output;

	float2 vps = float2(800.0f, 600.0f);

	float3 ndc = myVertInp[0].position.xyz / myVertInp[0].position.w;
	float2 p0 = float2((ndc.x + 1) * 0.5f * vps.x, (ndc.y + 1) * 0.5f * vps.y);	//going from ndc range [-1, 1] to 

	ndc = myVertInp[1].position.xyz / myVertInp[1].position.w;
	float2 p1 = float2((ndc.x + 1) * 0.5f * vps.x, (ndc.y + 1) * 0.5f * vps.y);

	ndc = myVertInp[2].position.xyz / myVertInp[2].position.w;
	float2 p2 = float2((ndc.x + 1) * 0.5f * vps.x, (ndc.y + 1) * 0.5f * vps.y);

	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);

	float alpha = acos( (b*b + c*c - a*a) / (2.0*b*c) );
	float beta = acos( (a*a + c*c - b*b) / (2.0*a*c) );

	float ha = abs( c * sin( beta ) );
	float hb = abs( c * sin( alpha ) );
	float hc = abs( b * sin( alpha ) );

	output.position = myVertInp[0].position;
	output.distance = float3(ha, 0, 0);
	OutputStream.Append(output);

	output.position = myVertInp[1].position;
	output.distance = float3(0, hb, 0);
	OutputStream.Append(output);

	output.position = myVertInp[2].position;
	output.distance = float3(0, 0, hc);
	OutputStream.Append(output);
    
    OutputStream.RestartStrip();
}
