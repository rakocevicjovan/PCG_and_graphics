#pragma once
#include "Math.h"
#include "ColFuncs.h"



// My preferred method, z project and unproject, in SVec4 w should be 1 (because of vectorized multiply)
inline static SVec4 unprojectPoint(SVec4 clipSpaceXYZ1, float clipSpaceW, const SMatrix& invProj)
{
	return SVec4::Transform((clipSpaceXYZ1 * clipSpaceW), invProj);
}



// My method, precalculate ray direction z and unproject the ray
inline static SVec3 viewRayDepthSliceIntersection(SVec3 rayDir, float vs_planeZ, const SMatrix& invProj)
{
	SPlane zPlane(SVec3(0, 0, 1), -vs_planeZ);

	SRay viewRay(SVec3(0.f), rayDir);	// No normalization, just shoot the ray, seems to be correct
	viewRay.direction = SVec3::Transform(viewRay.direction, invProj);

	SVec3 temp;
	Col::RayPlaneIntersection(viewRay, zPlane, temp);
	return temp;
}



// From the tutorial
inline static  SVec4 clipToView(SVec4 clip, SMatrix invProj)
{
	SVec4 view = SVec4::Transform(clip, invProj);	// View space transform
	return (view / view.w);							// Perspective division
}



// From the tutorial, use z = 0 and w = 1, then convert ray direction to view space, gives same results as above (I think?)
inline static SVec3 viewRayDepthSliceIntersection(float dirX, float dirY, float vs_planeZ, const SMatrix& invProj)
{
	SPlane zPlane(SVec3(0, 0, 1), -vs_planeZ);

	SVec4 vsDirection = clipToView(SVec4(dirX, dirY, 0.f, 1.f), invProj);
	SRay viewRay(SVec3(0.f), SVec3(&vsDirection.x));

	SVec3 temp;
	Col::RayPlaneIntersection(viewRay, zPlane, temp);
	return temp;
}



/* Taken from Doom presentation http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf page 5/58 */
inline static  float zSliceToViewDepth(float zNear, float zFar, uint8_t slice, uint8_t numSlices)
{
	float exponent = static_cast<float>(slice) / numSlices;
	return zNear * pow((zFar / zNear), exponent);
}



inline static  uint8_t viewDepthToZSlice(float n, float f, float viewDepth, float Sz)
{
	return log(viewDepth) * Sz / log(f / n) - Sz * log(n) / log(f / n);
}



inline static  uint8_t viewDepthToZSliceOpt(float sz_div_log_fdn, float log_n, float viewDepth)
{
	//log(viewDepth) * Sz / log(f / n)  - Sz * log(n) / log(f / n);		// Original
	//log(viewDepth) * Sz / log_fdn		- Sz * log_n / log_fdn;			// Remove constant logs
	//log(viewDepth) * Sz * inv_log_fdn - Sz * log_n * inv_log_fdn;		// Remove constant divisions
	//log(viewDepth) * sz_div_log_fdn	- sz_div_log_fdn * log_n;		// Sz * _inv_log_fdn can be precalculated					
	return sz_div_log_fdn * (log(viewDepth) - log_n);					// Extract sz_div_log_fdn from both sides
}



inline static float zViewToZClip(float zNear, float zFar, float viewDepth)
{
	return (zFar * (viewDepth - zNear)) / ((zFar - zNear) * viewDepth);
}



inline static float clipZToViewZ(float n, float f, float z)
{
	return (n*f) / (f + (n - f) * z);
}