#include "Perlin.h"
#include <random>

namespace Procedural
{

	Perlin::Perlin()
	{
	}



	Perlin::~Perlin()
	{
	}



	double Perlin::fade(double t) {
		return pow(t, 3) * ( t * (6 * t - 15) + 10 );	//6t5-15t4+10t3
	}



	double Perlin::perlin1d(double x) 
	{
		double left;
		double fract = modf(x, &left);	//expected value in left is a floored double
	}



	SVec2 Perlin::perlin2d(SVec2 pos)
	{
		//determine the square this point belongs to
		float left, bottom;
		float u = fade(modf(pos.x, &left));
		float v = fade(modf(pos.y, &bottom));

		//define the corners of the square
		SVec2 bottomLeft(left, bottom);
		SVec2 bottomRight(left + 1.0f, bottom);
		SVec2 topLeft(left, bottom + 1.0f);
		SVec2 topRight(left + 1.0f, bottom + 1.0f);

		//find gradients of the corners from the precomputed array
		SVec2 gradBL, gradBR, gradTL, gradTR;

		//calculate vectors from the corners to the position vector...
		SVec2 deltaBL = pos - bottomLeft;
		SVec2 deltaBR = pos - bottomRight;
		SVec2 deltaTL = pos - topLeft;
		SVec2 deltaTR = pos - topRight;

		//calculate dot products of the delta vectors with the gradient vectors
		float dotBl = deltaBL.Dot(gradBL);
		float dotBR = deltaBR.Dot(gradBR);
		float dotTL = deltaTL.Dot(gradTL);
		float dotTR = deltaTR.Dot(gradTR);

		int x1, x2;		//ok I'm really confused with the tutorial explanation of this part...
	}
		
}