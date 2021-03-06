#pragma once
#include "Math.h"
#include "Camera.h"

class Picker
{
public:

	static SRay generateRay(int wndW, int wndH, int mX, int mY, const Camera& cam)
	{
		SRay pickingRay;
		pickingRay.position = cam.GetPosition();
		
		SVec3 rayEnd(2.f * mX / wndW - 1.f, 1.f - 2.f *  mY / wndH, 1.f);			//back to ndc
		rayEnd = SVec3::Transform(rayEnd, cam.GetProjectionMatrix().Invert());		//back to view space
		rayEnd = SVec3::Transform(rayEnd, cam.GetViewMatrix().Invert());			//back to world space
		
		pickingRay.direction = (rayEnd - pickingRay.position);
		pickingRay.direction.Normalize();

		return pickingRay;
	}
};