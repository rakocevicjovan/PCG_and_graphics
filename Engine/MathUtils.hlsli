//holds useful math functions

float remap(float value, float min1, float max1, float min2, float max2)
{
	return min2 + ((value - min1) / (max1 - min1)) * (max2 - min2);
}


//from shadertoy, likely
float float2ToFloatRand1(in float2 seed)
{
	seed = frac(seed * float2(5.3983, 5.4427));
	seed += dot(seed.yx, seed.xy + float2(21.5351, 14.3137));
	return frac(seed.x * seed.y * 95.4337);
}


// Dave Hoskins - https://www.shadertoy.com/view/4djSRW
float float2ToFloatRand2(in float2 p)
{
	float3 p3 = frac(float3(p.xyx) * float3(443.897, 441.423, 437.195));
	p3 += dot(p3, p3.yzx + 19.19);
	return frac((p3.x + p3.y) * p3.z);
}



float2x2 makeRotMat(in float theta)
{
	float c = cos(theta);
	float s = sin(theta);
	return float2x2(c, -s, s, c);
}


float3 getSpherical(float3 inPos)
{
	float3 result;
	result.x = length(inPos);
	result.y = atan2(inPos.x, inPos.y);
	result.z = acos(inPos.z / result.x);
	return result;
}



float3 gridSpherical(float3 inSph, float d1, float d2)
{
	return float3(inSph.x, floor(inSph.y * d1), floor(inSph.z * d2));
}



float3 getCartesian(float3 inSph)
{
	return float3(
		inSph.x * cos(inSph.y) * sin(inSph.z),
		inSph.x * sin(inSph.y) * sin(inSph.z),
		inSph.x * cos(inSph.z));
}





float2 randVec2(float2 p)
{
	return float2(N2(p), N2(p.yx)) - .5;
}



float2 getCellStar(float2 cellID)
{
	//float t = iTime * .33;      //return vec2(sin(t * temp.x), cos(t * temp.y)) * .4; //moving
	float2 temp = randVec2(cellID);
	return float2(temp.x, temp.y) * .8;
}