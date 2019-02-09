#include "Geometry.h"
#include <numeric>

namespace Procedural
{


	Geometry::Geometry()
	{
	}


	Geometry::~Geometry()
	{
	}

	void Geometry::GenCircle(float radius, unsigned int subdivs)
	{
		positions.reserve(subdivs);
		std::iota(indices.begin(), indices.end(), subdivs);

		float deltaAngle = 2 * PI / (float)subdivs;
		float angle = 0;

		for (int i = 0; i < subdivs; ++i)
		{
			positions.push_back(SVec3(cos(angle), 0.f, sin(angle)) * radius);
			angle += deltaAngle;
		}

	}

	void Geometry::GenDisk(float radius, unsigned int subdivs, unsigned int circles, float decay)
	{
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