#pragma once
#include "Math.h"
#include "Camera.h"

class Picker
{
	SRay generateRay(int scrW, int scrH, const SVec2& scrPos, const Camera& cam)
	{
		SRay pickingRay;
		pickingRay.position = cam.GetPosition();

		return pickingRay;
	}
};