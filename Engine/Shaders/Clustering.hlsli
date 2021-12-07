
static const float GX = 30.;
static const float GY = 17.;	//16.875
static const float GZ = 16.;


/*
static const float GX = 2.;
static const float GY = 2.;
static const float GZ = 2.;
*/



/*
// Used for old setup with uint16_t packed lists

struct LightIndex
{
	uint index;

	uint getFirst()  { return (index >> 16); }			// get top half, shift bottom half to oblivion
	uint getSecond() { return (index & 0x0000FFFF); }	// mask out top half, get bottom half
};
*/

/*
struct OffsetCount
{
	// HOWEVER! WITH BINNING BEING AS IT IS, COUNTS WILL ALL BE ZERO! USE OFFSETS FOR RANGE!
	uint oc;
	/*
	

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
	return ( (n * f) / (f + (n - f) * z) );
}



int3 getClusterIndexTriplet(int2 ss_xy, float linearDepth, float n, float f)	//Useful for debug
{
	//int x = (ss_xy.x / 1920.f) * GX;
	//int y = GY - (ss_xy.y >> 6);	// Not correct as I use 17 slices, not 16.875 (would be better for wave coherency...)

	int x = (ss_xy.x >> 6);
	int y = ((1080.f - ss_xy.y) / 1080.f) * GY;
	int z = (log(linearDepth) * GZ / log(f / n)) - (GZ * log(n) / log(f / n));
	
	/* This does NOT fix the wrap around the right to the left edge of the screen, or whatever else is causing the bug...
	x = clamp(x, 0, 29);
	y = clamp(y, 0, 16);
	z = clamp(z, 0, 15);
	*/

	return int3(x, y, z);
}



int getClusterIndex(int2 ss_xy, float linearDepth, float n, float f)
{
	int3 xyz = getClusterIndexTriplet(ss_xy, linearDepth, n, f);

	return (xyz.z * GX * GY) + (xyz.y * GX) + xyz.x;
}


/*
int3 gcit(int2 ss_xy, float linearDepth, float n, float f)
{
	int x = (ss_xy.x / 1920.) * GX;
	int y = (ss_xy.y / 1080.) * GY;
	int z = log(linearDepth) * GZ / log(f / n) - GZ * log(n) / log(f / n);

	return int3(x, y, z);
}
*/


float SampleDigit(const in float n, const in float2 vUV)
{
	if (vUV.x < 0.0) return 0.0;
	if (vUV.y < 0.0) return 0.0;
	if (vUV.x >= 1.0) return 0.0;
	if (vUV.y >= 1.0) return 0.0;

	float data = 0.0;

	if (n < 0.5) data = 7.0 + 5.0*16.0 + 5.0*256.0 + 5.0*4096.0 + 7.0*65536.0;
	else if (n < 1.5) data = 2.0 + 2.0*16.0 + 2.0*256.0 + 2.0*4096.0 + 2.0*65536.0;
	else if (n < 2.5) data = 7.0 + 1.0*16.0 + 7.0*256.0 + 4.0*4096.0 + 7.0*65536.0;
	else if (n < 3.5) data = 7.0 + 4.0*16.0 + 7.0*256.0 + 4.0*4096.0 + 7.0*65536.0;
	else if (n < 4.5) data = 4.0 + 7.0*16.0 + 5.0*256.0 + 1.0*4096.0 + 1.0*65536.0;
	else if (n < 5.5) data = 7.0 + 4.0*16.0 + 7.0*256.0 + 1.0*4096.0 + 7.0*65536.0;
	else if (n < 6.5) data = 7.0 + 5.0*16.0 + 7.0*256.0 + 1.0*4096.0 + 7.0*65536.0;
	else if (n < 7.5) data = 4.0 + 4.0*16.0 + 4.0*256.0 + 4.0*4096.0 + 7.0*65536.0;
	else if (n < 8.5) data = 7.0 + 5.0*16.0 + 7.0*256.0 + 5.0*4096.0 + 7.0*65536.0;
	else if (n < 9.5) data = 7.0 + 4.0*16.0 + 7.0*256.0 + 5.0*4096.0 + 7.0*65536.0;

	float2 vPixel = floor(vUV * float2(4.0, 5.0));
	float fIndex = vPixel.x + (vPixel.y * 4.0);

	return floor(data / pow(2.0, fIndex)) % 2.0;
}



float PrintInt(const in float2 uv, const in float value)
{
	float res = 0.0;
	float maxDigits = 1.0 + ceil(log2(value) / log2(10.0));
	float digitID = floor(uv.x);
	if (digitID > 0.0 && digitID < maxDigits)
	{
		float digitVa = floor( value / pow(10.0, maxDigits - 1.0 - digitID) ) % 10.0;
		res = SampleDigit(digitVa, float2(frac(uv.x), uv.y));
	}

	return res;
}