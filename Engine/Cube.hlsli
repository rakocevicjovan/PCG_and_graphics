// This is reverse order from what dx11 expects, iterate using 35 - vertexID
static const float4 UNIT_CUBE[] =
{
float4(0.0f, 1.0f, 1.0f, 1.0f),
float4(0.0f, 0.0f, 1.0f, 1.0f),
float4(0.0f, 0.0f, 0.0f, 1.0f),
float4(1.0f, 1.0f, 1.0f, 1.0f),
float4(1.0f, 0.0f, 1.0f, 1.0f),
float4(0.0f, 0.0f, 1.0f, 1.0f),
float4(1.0f, 1.0f, 0.0f, 1.0f),
float4(1.0f, 0.0f, 0.0f, 1.0f),
float4(1.0f, 0.0f, 1.0f, 1.0f),
float4(0.0f, 1.0f, 0.0f, 1.0f),
float4(0.0f, 0.0f, 0.0f, 1.0f),
float4(1.0f, 0.0f, 0.0f, 1.0f),
float4(0.0f, 0.0f, 1.0f, 1.0f),
float4(1.0f, 0.0f, 1.0f, 1.0f),
float4(1.0f, 0.0f, 0.0f, 1.0f),
float4(1.0f, 1.0f, 1.0f, 1.0f),
float4(0.0f, 1.0f, 1.0f, 1.0f),
float4(0.0f, 1.0f, 0.0f, 1.0f),
float4(0.0f, 1.0f, 0.0f, 1.0f),
float4(0.0f, 1.0f, 1.0f, 1.0f),
float4(0.0f, 0.0f, 0.0f, 1.0f),
float4(0.0f, 1.0f, 1.0f, 1.0f),
float4(1.0f, 1.0f, 1.0f, 1.0f),
float4(0.0f, 0.0f, 1.0f, 1.0f),
float4(1.0f, 1.0f, 1.0f, 1.0f),
float4(1.0f, 1.0f, 0.0f, 1.0f),
float4(1.0f, 0.0f, 1.0f, 1.0f),
float4(1.0f, 1.0f, 0.0f, 1.0f),
float4(0.0f, 1.0f, 0.0f, 1.0f),
float4(1.0f, 0.0f, 0.0f, 1.0f),
float4(0.0f, 0.0f, 0.0f, 1.0f),
float4(0.0f, 0.0f, 1.0f, 1.0f),
float4(1.0f, 0.0f, 0.0f, 1.0f),
float4(1.0f, 1.0f, 0.0f, 1.0f),
float4(1.0f, 1.0f, 1.0f, 1.0f),
float4(0.0f, 1.0f, 0.0f, 1.0f),
};

// Clockwise order: 
// 0, 2, 3
// 3, 1, 0
// ... cant be bothered...
// 
// 
// 
// 
// 

//static const float4 UNIT_CUBE_INDEXED[] =
//{
//float4(-1.0, -1.0, -1.0, 1.0),	//CBL
//float4(-1.0, -1.0, +1.0, 1.0),	//CBR
//float4(-1.0, +1.0, -1.0, 1.0),	//CTL
//float4(-1.0, +1.0, +1.0, 1.0),	//CTR
//float4(+1.0, -1.0, -1.0, 1.0),	//FBL
//float4(+1.0, -1.0, +1.0, 1.0),	//FBR
//float4(+1.0, +1.0, -1.0, 1.0),	//FTL
//float4(+1.0, +1.0, +1.0, 1.0),	//FTR
//};