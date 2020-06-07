#pragma once
#include "Math.h"



namespace Curves
{
	// Could be a template, it would work for scalars, 1d, 2d and 3d vectors
	struct Hermite
	{
		SVec3 p0, p1;
		SVec3 tg0, tg1;

		SVec3 getCubicHermite(float t)
		{
			float t2 = t * t;
			float t3 = t2 * t;

			SVec3 a = (2.f * t3 - 3.f * t2 + 1.f) * p0;
			SVec3 b = (t3 - 2.f * t2 + t) * tg0;

			SVec3 c = (-2.f * t3 + 3.f * t2) * p1;
			SVec3 d = (t3 - t2) * tg1;

			return a + b + c + d;
		}
	};

	



};