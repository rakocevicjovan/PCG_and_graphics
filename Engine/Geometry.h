#pragma once
#include <vector>
#include "Math.h"

namespace Procedural
{

	class Geometry
	{
	public:

		std::vector<SVec3> positions;
		std::vector<SVec2> texCoords;
		std::vector<SVec3> normals;
		std::vector<SVec3> tangents;
		std::vector<unsigned int> indices;

		Geometry();
		~Geometry();

		void GenRectangle(float halfWidth, float halfHeight, bool vertical = true);
		void GenBox(SVec3 dims, bool genTangents = false);
		void GenUVCircle(float radius, unsigned int subdivs);
		void GenUVDisk(float radius, unsigned int subdivs, float thiccness);
		void GenHalo(float radius, float innerRadius, unsigned int subdivs);
		void GenHelix(float angle, float length, float width, unsigned int subdivs, float xScale = 1.f, float zScale = 1.f);
		void GenTube(float radius, float height, UINT subdivsRadial, UINT rows, float minRadiusPerc = 1.f);
		void GenSphere(float radius);
		void GenHexaprism(float radius, float thiccness);
		std::vector<Geometry> GenHexGrid(float radius, float thiccness, UINT numHexes);
		void Clear();
	};
}