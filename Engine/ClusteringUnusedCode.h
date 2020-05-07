#pragma once
#include "ClusteringMath.h"
#include "Camera.h"
#include "Light.h"
#include "IMGUI/imgui.h"
#include "ClusterManager.h"


/* Really all kinds of things made along the way, useful in case of cluster debugging as it produces explicit cluster bounds. */

struct ClusterNode
{
	SVec4 _min;
	SVec4 _max;

	ClusterNode(SVec3 min, SVec3 max) : _min(min), _max(max)
	{
		_min.w = _max.w = 1.f;
	}
};



void buildClipSpacePlanes(const Camera& cam, std::vector<SPlane>& planes, std::array<UINT, 3> gridDims)
{
	planes.reserve(gridDims[0] + gridDims[1] + gridDims[2] + 3);	// Because it's a number of edge planes, not cells

	float zNear = cam._frustum._zn;
	float zFar = cam._frustum._zf;

	SMatrix invProj = cam.GetProjectionMatrix().Invert();

	// Slice widths
	float w = 2. / gridDims[0];
	float h = 2. / gridDims[1];

	SVec4 plane;


	plane = SVec4(0., 0., 1., 0.);

	for (int zSlice = 0; zSlice <= gridDims[2]; ++zSlice)
	{
		float zV = zSliceToViewDepth(zNear, zFar, zSlice, gridDims[2]);
		plane.w = zViewToZClip(zNear, zFar, zV);
		planes.push_back(plane);
	}


	plane = SVec4(1., 0., 0., 0.);

	for (int i = 0; i <= gridDims[0]; ++i)
	{
		plane.w = i * w - 1.f;
		planes.push_back(plane);
	}


	plane = SVec4(0., 1., 0., 0.);

	for (int j = 0; j <= gridDims[1]; ++j)
	{
		plane.w = j * h - 1.f;
		planes.push_back(plane);
	}
}



void buildGrid(const Camera& cam, std::array<UINT, 3> gridDims)
{
	float zNear = cam._frustum._zn;
	float zFar = cam._frustum._zf;
	SMatrix invProj = cam.GetProjectionMatrix().Invert();

	float w = 2. / gridDims[0];
	float h = 2. / gridDims[1];

	SVec3 min, max;
	float xL, xR, yB, yT;
	float n, nV, f, fV;

	for (int zSlice = 0; zSlice < gridDims[2]; ++zSlice)
	{
		nV = zSliceToViewDepth(zNear, zFar, zSlice, gridDims[2]);
		n = zViewToZClip(zNear, zFar, nV);

		fV = zSliceToViewDepth(zNear, zFar, zSlice + 1u, gridDims[2]);	// Get required linear depth according to slice
		f = zViewToZClip(zNear, zFar, fV);							// Transform it into projected Z

		min.z = nV;
		max.z = fV;

		for (int i = 0; i < gridDims[0]; ++i)
		{
			xL = i * w - 1.f;
			xR = xL + w;

			for (int j = 0; j < gridDims[1]; ++j)
			{
				yB = j * h - 1.f;
				yT = yB + h;

#ifdef VIEW_SPACE_CLUSTERS			
				// All of this is just there to get to view space, which is only important IF we are culling in it
				// Otherwise, the corners of each AABB (frustum in clip space) are known already from simple math above

				SVec4 lbnView = unprojectPoint(SVec4(xL, yB, n, 1.), nV, invProj);
				SVec4 lbfView = unprojectPoint(SVec4(xL, yB, f, 1.), fV, invProj);

				SVec4 trnView = unprojectPoint(SVec4(xR, yT, n, 1.), nV, invProj);
				SVec4 trfView = unprojectPoint(SVec4(xR, yT, f, 1.), fV, invProj);

				min.x = min(lbnView.x, lbfView.x);
				min.y = min(lbnView.y, lbfView.y);

				max.x = max(trnView.x, trfView.x);
				max.y = max(trnView.y, trfView.y);

				_grid.emplace_back(min, max);

				// Alternative method examples, both cases ray points to zNear! But it intersects with further plane
				// lbnView = viewRayDepthSliceIntersection(SVec3(xL, yB, zNear), nV, invProj);
				// trfView = viewRayDepthSliceIntersection(SVec3(xR, yT, zNear), fV, invProj);

				// Tutorial author's method, same result as above, and same as my no intersection method
				//lbnView = viewRayDepthSliceIntersection(xL, yB, nV, invProj);
#endif
			}
		}
	}
}



void renderSphereProjections(std::vector<PLight>& culledList, const Camera& cam)
{
	char windowName[20];

	for (int i = 0; i < min(culledList.size(), 1); i++)	//_culledList.size()
	{
		sprintf(windowName, "SPHERE_DEBUG %d", i);

		SVec3 ws_lightPos(culledList[i]._posRange);																				// OK
		SVec4 viewPosRange = Math::fromVec3(SVec3::Transform(ws_lightPos, cam.GetViewMatrix()), culledList[i]._posRange.w);	// OK
		SVec4 mm = ClusterManager::getProjectedRectangle(viewPosRange, 1., 1000., cam.GetProjectionMatrix());

		SVec2 p = SVec2(mm.x, mm.w) + SVec2(1.f);	//-1, 1 to 0, 2
		p *= 0.5f;		//0, 2 to 0, 1
		p.y = 1. - p.y;	// flip y
		p *= SVec2(1920., 1080.);

		SVec2 s = SVec2(abs(mm.z - mm.x), abs(mm.w - mm.y)) * SVec2(1920., 1080.) * 0.5f;

		ImVec2 pos(p.x, p.y);	//needs max y actually, so mm.z
		ImVec2 size(s.x, s.y);

		ImGui::PushID(i);

		ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(size, ImGuiCond_Always);

		ImGui::Begin(windowName, false);
		ImGui::End();

		ImGui::PopID();
	}
}