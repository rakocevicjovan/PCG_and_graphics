cbuffer FrustumData : register(b5)
{
    float4 plane[6];
};

StructuredBuffer<float4> spheres : register(t0);
RWStructuredBuffer<float> result : register(u0);

bool SphereInsidePlane(float4 plane, float4 sphere)
{
    float spCenterToNormalProjection = dot(sphere.xyz, plane.xyz);
    return (spCenterToNormalProjection + plane.w + sphere.r > 0.f);
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex)
{
    uint index = (dispatchThreadID.x * 16) + dispatchThreadID.y;
    
    for (int i = 0; i < 6; ++i)
    {
        if (!SphereInsidePlane(plane[i], spheres[index]))
            return;
    }
    result[index] = 1.f;
}