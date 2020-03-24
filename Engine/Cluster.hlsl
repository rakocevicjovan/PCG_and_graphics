#define MAX_LIGHTS  (1024.f)
#define TILE_SIZE   (32.f)
#define INV_SIZE    (1.f / TILE_SIZE)
#define RX			(1280.f)
#define RY			(720.f)


struct LightSBB
{
	float4 posRadius;
};

struct AABB
{
	float4 min;
	float4 max;
};

// Input structured buffer, contains positions of all point lights and spotlights (which will be clustered the same way...)
StructuredBuffer<LightSBB> : register(b10) lightBuffer;


uint getTilesXCount()
{
	return uint((RX + TILE_SIZE - 1) * INV_SIZE);
}



uint getTilesYCount()
{
	return uint((RY + TILE_SIZE - 1) * INV_SIZE);
}



[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	return 0;
}