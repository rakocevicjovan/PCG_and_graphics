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
		std::vector<SVec3> normals;
		std::vector<unsigned int> indices;

		Geometry();
		~Geometry();

		void GenRectangle(float halfWidth, float halfHeight, bool vertical = true);
		void GenBox(SVec3 halfDims);
		void GenUVCircle(float radius, unsigned int subdivs);
		void GenUVDisk(float radius, unsigned int subdivs, float thiccness);
		void GenHalo(float radius, float innerRadius, unsigned int subdivs);
		void GenHelix(float angle, float length, float width, unsigned int subdivs, float xScale = 1.f, float zScale = 1.f);
		void GenTube(float radius, float height, UINT subdivsRadial, UINT rows, float decay = 1.f);
		void GenSphere(float radius);
		
		void Clear();
	};

}