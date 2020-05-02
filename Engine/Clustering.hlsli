struct PLight
{
	float4 rgbi;
	float4 posRange;
};

struct LightIndex
{
	float i;
};


struct OffsetCount
{
	// HOWEVER! WITH BINNING BEING AS IT IS, COUNTS WILL ALL BE ZERO! USE OFFSETS FOR RANGE!
	int oc;

	uint getOffset()
	{
		return (oc >> 16);		// shift out the least significant half, obtain most significant half (uint16_t offset on cpu)
	}

	uint getCount()
	{
		return (oc & 0xFFFF);	// most significant half is masked out, obtain least significant half
	}

	uint2 getPair()
	{
		uint o = getOffset();	// shift out the least significant half, obtain most significant half (uint16_t offset on cpu)
		uint c = getCount();	// most significant half is masked out, obtain least significant half
		return uint2(o, c);
	}
};





float zToViewSpace(float z, float n, float f)
{
	return (n * f) / (f + (n - f) * z);
}



int getClusterIndex(float2 ss_xy, float linearDepth, float n, float f)
{
	int x = ss_xy.x >> 6;
	int y = ss_xy * (1. / 17.);
	int z = log(depth) * 16. / log(f / n) - 16. * log(n) / log(f / n);

	return z * 30. * 17. + y * 30. + x;
}