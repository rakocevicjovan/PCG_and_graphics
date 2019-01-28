#include "Perlin.h"
#include "Chaos.h"

namespace Procedural
{

	Perlin::Perlin()
	{
		hashTable.resize(512);
		std::copy_n(hashTable.begin(), 256, hashTable.begin() + 256);
	}



	Perlin::~Perlin()
	{
	}



	double Perlin::fade(double t)
	{
		return pow(t, 3) * (t * (6 * t - 15) + 10);	//6t5-15t4+10t3
	}


	//not implemented
	double Perlin::perlin1d(double x)
	{
		double left;
		double fract = modf(x, &left);	//expected value in left is a floored double

		return 0;
	}



	float Perlin::perlin2d(SVec2 pos)
	{
		//determine the square this point belongs to
		float left, bottom;
		float u = fade(modf(pos.x, &left));
		float v = fade(modf(pos.y, &bottom));
		int iLeft = round(left);
		int iBottom = round(bottom);

		//define the corners of the square
		SVec2 bottomLeft(left, bottom);
		SVec2 bottomRight(left + 1.0f, bottom);
		SVec2 topLeft(left, bottom + 1.0f);
		SVec2 topRight(left + 1.0f, bottom + 1.0f);

		//find gradients of the corners from the precomputed array
		SVec2 gradBL = gradients2D[hash2D(iLeft, iBottom) % 8];
		SVec2 gradBR = gradients2D[hash2D(iLeft + 1, iBottom) % 8];
		SVec2 gradTL = gradients2D[hash2D(iLeft, iBottom + 1) % 8];
		SVec2 gradTR = gradients2D[hash2D(iLeft + 1, iBottom + 1) % 8];

		//calculate vectors from the corners to the position vector...
		SVec2 deltaBL = pos - bottomLeft;
		SVec2 deltaBR = pos - bottomRight;
		SVec2 deltaTL = pos - topLeft;
		SVec2 deltaTR = pos - topRight;

		//calculate dot products of the delta vectors with the gradient vectors
		float dotBL = deltaBL.Dot(gradBL);
		float dotBR = deltaBR.Dot(gradBR);
		float dotTL = deltaTL.Dot(gradTL);
		float dotTR = deltaTR.Dot(gradTR);

		//interpolate between results
		float bottomInterpolated = dotBL * u + dotBR * (1 - u);
		float topInterpolated = dotTL * u + dotTR * (1 - u);

		return bottomInterpolated * v + topInterpolated * (1 - v);
	}



	inline int Perlin::hash2D(int x, int y) const
	{
		return hashTable[hashTable[x] + y];
	}



	std::vector<SVec2> Perlin::gradients2D =
	{
	SVec2(0.707f, 0.707f),
	SVec2(0.707f, -0.707f),
	SVec2(-0.707f, 0.707f),
	SVec2(-0.707f, -0.707f),
	SVec2(0.f, 1.f),
	SVec2(0.f, -1.f),
	SVec2(1.f, 0.f),
	SVec2(-1.f, 0.f)
	};



	std::vector<int> Perlin::hashTable =
	{
		151, 160, 137, 91, 90, 15,
		131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
		190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
		88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
		77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
		102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
		135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
		5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
		223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
		129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
		251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
		49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
		138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
	};
}