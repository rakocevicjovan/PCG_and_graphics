


float rgbToLuminance(float3 col)
{
	return dot(col, float3(0.2126f, 0.7152f, 0.0722f));
}


float3 changeLuminance(float3 col, float newLuminance)
{
	float inLuminance = rgbToLuminance(col);
	return col * (newLuminance / inLuminance);
}


// RGB versions
float3 TMO_reinhardSimple(float3 col)
{
	return (col / ((float3)(1.f) + col));
}


// Accounts for maximum radiance value in the scene
float3 TMO_reinhard(float3 col, float3 cMax)
{
	float3 num = col * ((float3)(1.f) + (col / (cMax * cMax)));
	float3 denom = (float3)(1.f) + col;
	return num / denom;
}