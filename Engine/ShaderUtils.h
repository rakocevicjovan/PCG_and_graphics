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