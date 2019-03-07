#include "Geometry.h"

namespace Procedural
{


	Geometry::Geometry()
	{
	}


	Geometry::~Geometry()
	{
	}


	//2D geometry generater in the xz plane
	void Geometry::GenRectangle(float hw, float hh)
	{
		positions = {SVec3(-hw, 0, hh), SVec3(hw, 0, hh), SVec3(-hw, 0, -hh), SVec3(hw, 0, -hh)};
		indices = { 0, 1, 2, 2, 1, 3 };
	}



	void Geometry::GenUVCircle(float radius, unsigned int subdivs)
	{
		positions.reserve(subdivs + 1);
		indices.reserve(subdivs * 3);

		positions.push_back(SVec3(0, 0, 0));

		float deltaAngle = 2 * PI / (float)subdivs;
		float angle = 0;

		for (int i = 1; i < subdivs; ++i)
		{
			positions.push_back(SVec3(cos(angle), 0.f, sin(angle)) * radius);
			indices.push_back(0);
			indices.push_back(i);
			indices.push_back(i + 1);
			angle += deltaAngle;
		}
		indices.back() = 1;

	}



	void Geometry::GenUVDisk(float radius, unsigned int subdivs, float thiccness)
	{
		GenUVCircle(radius, subdivs);
		unsigned int oldVertCount = positions.size();
		positions.reserve(2 * oldVertCount);
		
		float halfThiccness = thiccness * 0.5f;

		for(auto& p : positions)
		{
			p.y -= halfThiccness;
			positions.push_back(SVec3(p.x, halfThiccness, p.z));
		}

		unsigned int oldSize = indices.size();
		indices.reserve(2 * oldSize + subdivs * 6);
		
		for(int i = 0 ; i < oldSize; ++i)
		{
			indices.push_back(indices[i] + oldVertCount);
		}
		
		for (int i = 1; i < oldVertCount; ++i)
		{
			indices.push_back(i + oldVertCount);
			indices.push_back(i + oldVertCount + 1);
			indices.push_back(i);
			indices.push_back(i);
			indices.push_back(i + oldVertCount + 1);
			indices.push_back(i + 1);
		}
	}



	void Geometry::GenHalo(float radius, float innerRadius, unsigned int subdivs, unsigned int circles, float decay)
	{
	}



	void Geometry::GenHelix(float angle, float length, float width, unsigned int subdivs, float xScale, float zScale)
	{
		positions.reserve(subdivs);

		float t = 0;

		float xcf = width * xScale, zcf = width * zScale;

		for (int i = 0; i < subdivs; ++i)
		{
			t += length / (float)i;
			positions.push_back(SVec3(xcf* cos(t), t, zcf * sin(t)));
		}
	}

}