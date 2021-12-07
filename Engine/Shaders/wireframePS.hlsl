struct Pyxis
{
    float4 position : SV_POSITION;
	noperspective float3 distance : DIST;
};


float4 main(Pyxis input) : SV_TARGET
{
	float minDist = min(length(input.distance[0]), min(length(input.distance[1]), length(input.distance[2])));

	float alpha = .1f - 0.1f * minDist;

	if(alpha <= 0.0f)
		discard;

	float4 colour = float4(1. - alpha, 1. - alpha, 1. - alpha, alpha);
	
	return colour;
}