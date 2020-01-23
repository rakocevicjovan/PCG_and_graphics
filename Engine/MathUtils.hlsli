//holds useful math functions


float remap(float value, float min1, float max1, float min2, float max2)
{
	return min2 + ((value - min1) / (max1 - min1)) * (max2 - min2);
}


//from shadertoy, likely
float randomizer(in float2 seed)
{
	seed = frac(seed * float2(5.3983, 5.4427));
	seed += dot(seed.yx, seed.xy + float2(21.5351, 14.3137));
	return frac(seed.x * seed.y * 95.4337);
}


float2x2 makeRotMat(in float theta)
{
	float c = cos(theta);
	float s = sin(theta);
	return float2x2(c, -s, s, c);
}