//holds SDFs for various useful shapes

float sdTorus(float3 p, float2 t)
{
	return length(float2(length(p.xz) - t.x, p.y)) - t.y;
}


float sdCylinder(float3 p, float3 c)
{
	return length(p.xz - c.xy) - c.z;
}


float3 opTwist(in float3 p, uniform TWISTER)
{
	float nani = TWISTER * p.y - elapsed * .66;
	float c = cos(nani);
	float s = sin(nani);
	float2x2 rotoMato = float2x2(c, -s, s, c);
	return float3(mul(p.xz, rotoMato), p.y);
}