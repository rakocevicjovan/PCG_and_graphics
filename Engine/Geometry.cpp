#include "Geometry.h"
#include "GeometricPrimitive.h"
#include "Chaos.h"

namespace Procedural
{


	Geometry::Geometry()
	{
	}


	Geometry::~Geometry()
	{
	}


	//2D geometry generater in the xz plane
	void Geometry::GenRectangle(float hw, float hh, bool vertical)
	{
		if(!vertical)
			positions = { SVec3(-hw, 0, hh), SVec3(hw, 0, hh), SVec3(-hw, 0, -hh), SVec3(hw, 0, -hh) };
		else
			positions = { SVec3(-hw, hh, 0), SVec3(hw, hh, 0), SVec3(-hw, -hh, 0), SVec3(hw, -hh, 0) };
		indices = { 0, 1, 2, 2, 1, 3 };
	}



	void Geometry::GenBox(SVec3 hd)
	{

		std::vector<DirectX::VertexPositionNormalTexture> verts;
		std::vector<uint16_t> inds;
		DirectX::GeometricPrimitive::CreateBox(verts, inds, DirectX::XMFLOAT3(hd.x, hd.y, hd.z), false, false);

		positions.reserve(verts.size());
		for (auto v : verts)
		{
			positions.push_back(v.position);
			normals.push_back(v.normal);
		}

		indices.reserve(inds.size());
		for (auto i : inds)
			indices.push_back(i);
	}



	void Geometry::GenUVCircle(float radius, unsigned int subdivs)
	{
		positions.reserve(subdivs + 1);
		indices.reserve(subdivs * 3);

		positions.push_back(SVec3(0, 0, 0));

		float deltaAngle = 2. * PI / (float)subdivs;
		float angle = 0;

		for (int i = 0; i < subdivs; ++i)
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



	void Geometry::GenHalo(float radius, float innerRadius, unsigned int subdivs)
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
	


	void Geometry::GenTube(float radius, float height, UINT subdivsRadial, UINT rows, float decay)
	{
		positions.reserve(rows * subdivsRadial);
		indices.reserve(subdivsRadial * 2 * 3 * (rows - 1));	//number of faces per column, times 2 for two subrows, times face rows, times 3

		float subdivHeight = height / float(rows - 1);

		//create a ring
		float angle = 0.f;
		float dAngle = 2. * PI / float(subdivsRadial);

		float radiusDelta = radius * (1.f - decay);

		std::vector<float> sines, cosines;
		sines.reserve(subdivsRadial);
		cosines.reserve(subdivsRadial);

		//avoid recalculating sines/cosines
		for (unsigned int j = 0; j < subdivsRadial; ++j)
		{
			cosines.push_back(cos(angle));
			sines.push_back(sin(angle));
			angle += dAngle;
		}


		//iterate rows to create the vertices
		for (UINT i = 0; i < rows; ++i)
		{
			float adjRadius = radius - Math::smoothstep(0, rows - 1.f, i) * radiusDelta;

			for (UINT j = 0; j < subdivsRadial; ++j)
			{
				normals.push_back(SVec3(cosines[j], 0.f, sines[j]));
				positions.push_back(SVec3(normals.back().x * adjRadius, i * subdivHeight, normals.back().z * adjRadius));
				
				//and link them with indices - except the last row, because the current row already takes the next one into account to build the index buffer
				if (i == rows - 1) continue;

				UINT indexAbove = (i + 1) * subdivsRadial + j;
				UINT indexHere = i * subdivsRadial + j;
				
				if (j == subdivsRadial - 1)
				{
					indices.insert(indices.end(), { indexAbove, (i + 1) * subdivsRadial, indexHere });		//11, 6, 5
					indices.insert(indices.end(), { indexHere, (i + 1) * subdivsRadial, i * subdivsRadial });	//5, 6, 0
				}
				else
				{
					indices.insert(indices.end(), { indexAbove, indexAbove + 1, indexHere });
					indices.insert(indices.end(), { indexHere, indexAbove + 1, indexHere + 1 });
				}
			}
		}
	}

	void Geometry::GenSphere(float radius)
	{
		std::vector<DirectX::VertexPositionNormalTexture> verts;
		std::vector<uint16_t> inds;
		DirectX::GeometricPrimitive::CreateSphere(verts, inds, radius * 2.f, 4u, false, false);	// DirectX::XMFLOAT3(1.f / 2.f, 2.f / 2.f, 3.f / 2.f)

		positions.reserve(verts.size());
		for (auto v : verts)
		{
			positions.push_back(v.position);
			normals.push_back(v.normal);
		}
			
		indices.reserve(inds.size());
		for (auto i : inds)
			indices.push_back(i);
	}

	void Geometry::Clear()
	{
		positions.clear();
		normals.clear();
		indices.clear();
	}

}