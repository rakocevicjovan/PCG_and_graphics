#pragma once
#include "Math.h"
#include <vector>

class Camera;

class Frustum
{
public:
	//left, right, bottom, top, near, far
	std::vector<SPlane> planes;

	
	Frustum() {}
	Frustum(const SMatrix& pm);

	//from the fast extraction paper...
	void update(const SMatrix& camMatrix);
};