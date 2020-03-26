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
StructuredBuffer<LightSBB> lightBuffer : register(t10);

//RWStructuredBuffer<StructNameHere> BufferOut : register(u0);

uint getTilesXCount()
{
	return uint((RX + TILE_SIZE - 1) * INV_SIZE);
}



uint getTilesYCount()
{
	return uint((RY + TILE_SIZE - 1) * INV_SIZE);
}


/*
	X axis: 1920px split into 64px blocks	=> 1920 / 64 = 30 (30 blocks of 64 px width)
	Y axis: 1080px  						=> 1080 / 36 = 30 (30 blocks of 36 px width, to keep consistent with 30 blocks)
	Z axis: 32 slices 						=> easy, 32


	// Official docs on compute shaders (very meh tbh)
	https://docs.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader

	// Better explanation
	http://www.codinglabs.net/tutorial_compute_shaders_filters.aspx
*/

/*
uint3 groupID : SV_GroupID
- Index of the group within the dispatch for each dimension

uint3 groupThreadID : SV_GroupThreadID
- Index of the thread within the group for each dimension

uint groupIndex : SV_GroupIndex
- A sequential index within the group that starts from 0 top left back and goes on to bottom right front

uint3 dispatchThreadID : SV_DispatchThreadID
- Global thread index within the whole dispatch
*/

float sqDistPointAABB(float3 min, float3 max, float3 p)
{
	float3 closestPoint;
	closestPoint.x = clamp(min.x, max.x, p.x);
	closestPoint.y = clamp(min.y, max.y, p.y);
	closestPoint.z = clamp(min.z, max.z, p.z);

	float3 delta = p - closestPoint;

	return dot(delta, delta);
}

bool aabbSphereIntersection(float3 min, float3 max, float4 sphere)
{

}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

}