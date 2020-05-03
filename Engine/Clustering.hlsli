
static const float GX = 30.;
static const float GY = 17.;
static const float GZ = 16.;


/*
static const float GX = 2.;
static const float GY = 2.;
static const float GZ = 2.;
*/



struct PLight
{
	float4 rgbi;
	float4 posRange;
};

/*

struct LightIndex
{
	uint index;

	//uint getIndex() { return (index >> 16); }
};


struct OffsetCount
{
	// HOWEVER! WITH BINNING BEING AS IT IS, COUNTS WILL ALL BE ZERO! USE OFFSETS FOR RANGE!
	uint oc;



	/*
	// What the hell am I completely wrong about everything??

	// shift out the least significant half, obtain most significant half (uint16_t offset on cpu)
	//uint getOffset() { return (oc >> 16); }

	// most significant half is masked out, obtain least significant half
	//uint getCount() { return (oc & 0xFFFF);	}

	//uint2 getPair()
	//{
	//	uint o = getOffset();	// shift out the least significant half, obtain most significant half (uint16_t offset on cpu)
	//	uint c = getCount();	// most significant half is masked out, obtain least significant half
	//	return uint2(o, c);
	//}
	
};
*/


float zToViewSpace(float z, float n, float f)
{
	return (n * f) / (f + (n - f) * z);
}



int3 getClusterIndexTriplet(int2 ss_xy, float linearDepth, float n, float f)	//Useful for debug
{
	int x = (ss_xy.x >> 6);
	//int y = GY - (ss_xy.y >> 6);	// Not quite correct I think?
	int y = GY - ((ss_xy.y / 1080.) * GY);
	int z = log(linearDepth) * GZ / log(f / n) - GZ * log(n) / log(f / n);

	return int3(x, y, z);
}



int getClusterIndex(int2 ss_xy, float linearDepth, float n, float f)
{
	int3 xyz = getClusterIndexTriplet(ss_xy, linearDepth, n, f);

	return xyz.z * GX * GY + xyz.y * GX + xyz.x;
}



int3 gcit(int2 ss_xy, float linearDepth, float n, float f)
{
	int x = (ss_xy.x / 1920.) * GX;
	int y = (ss_xy.y / 1080.) * GY;
	int z = log(linearDepth) * GZ / log(f / n) - GZ * log(n) / log(f / n);

	return int3(x, y, z);
}