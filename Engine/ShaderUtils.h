#pragma once
#include "Math.h"


float plaIntersect(SVec3 ro, SVec3 rd, SVec4 p)
{
	return -(ro.Dot(SVec3(p.x, p.y, p.z)) + p.w) / rd.Dot(SVec3(p.x, p.y, p.z));
}



SVec2 RaySphereInt(SVec3 ro, SVec3 rd, SVec3 sc, float r)
{
	float t = (sc - ro).Dot(rd);     //project SCtoRO vector onto ray direction, get t of closest point
	SVec3 p = ro + rd * t;         //get that closest point, easy...

	SVec3 rToC = sc - p;           //vector from ray to sphere center...
	float y = rToC.Dot(rToC);      //squared distance between closest point on ray and center of sphere

	float x = r * r - y;            //Pythagorean theorem, get squared distance from middle point to shell
	if (x < 0.f)
		return SVec2(0.f, 0.f);

	x = sqrt(x);
	return SVec2(t - x, t + x);
}


namespace Sebh
{

	//taken from Sebastien Hillaire at https://github.com/sebh/TileableVolumeNoise/blob/master/TileableVolumeNoise.cpp
	float hash(float n)
	{
		return Math::fract(sin(n + 1.951f) * 43758.5453f);
	}


	//taken from Sebastien Hillaire at https://github.com/sebh/TileableVolumeNoise/blob/master/TileableVolumeNoise.cpp
	float noise(const SVec3& x)
	{
		SVec3 p = Math::floor3(x);
		SVec3 f = Math::fract3(x);

		f = f * f*(SVec3(3.0f) - SVec3(2.0f) * f);
		float n = p.x + p.y*57.0f + 113.0f*p.z;
		return Math::lerp(
			Math::lerp(
				Math::lerp(hash(n + 0.0f), hash(n + 1.0f), f.x),
				Math::lerp(hash(n + 57.0f), hash(n + 58.0f), f.x),
				f.y),
			Math::lerp(
				Math::lerp(hash(n + 113.0f), hash(n + 114.0f), f.x),
				Math::lerp(hash(n + 170.0f), hash(n + 171.0f), f.x),
				f.y),
			f.z);
	}

	//taken from Sebastien Hillaire at https://github.com/sebh/TileableVolumeNoise/blob/master/TileableVolumeNoise.cpp
	float Cells(const SVec3& p, float cellCount)
	{
		const SVec3 pCell = p * cellCount;
		float d = 1.0e10;
		for (int xo = -1; xo <= 1; xo++)
		{
			for (int yo = -1; yo <= 1; yo++)
			{
				for (int zo = -1; zo <= 1; zo++)
				{
					SVec3 tp = Math::floor3(pCell) + SVec3(xo, yo, zo);

					tp = pCell - tp - SVec3(noise(SVec3(fmodf(tp.x, cellCount), fmodf(tp.y, cellCount), fmodf(tp.z, cellCount))));

					d = std::min(d, tp.Dot(tp));
				}
			}
		}
		d = std::fminf(d, 1.0f);
		d = std::fmaxf(d, 0.0f);
		return d;
	}
}