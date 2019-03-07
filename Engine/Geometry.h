#pragma once
#include <vector>
#include "Math.h"

namespace Procedural
{

	using SPlane = DirectX::SimpleMath::Plane;

	class Geometry
	{
	public:

		std::vector<SVec3> positions;
		std::vector<unsigned int> indices;

		Geometry();
		~Geometry();

		void GenRectangle(float halfWidth, float halfHeight);

		void GenUVCircle(float radius, unsigned int subdivs);
		void GenUVDisk(float radius, unsigned int subdivs, float thiccness);
		void GenHalo(float radius, float innerRadius, unsigned int subdivs, unsigned int circles, float decay = 1.f);
		void GenHelix(float angle, float length, float width, unsigned int subdivs, float xScale = 1.f, float zScale = 1.f);
	};

}