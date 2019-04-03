#include "Geometry.h"
#include "GeometricPrimitive.h"
#include "Chaos.h"
#include <map>

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



	SVec3 calculateTangent(const std::vector<SVec3>& positions, const std::vector<SVec2>& texCoords, int ind0, int ind1, int ind2)
	{
		SVec3 tangent;
		SVec3 edge1, edge2;
		SVec2 duv1, duv2;

		//Find first texture coordinate edge 2d vector
		edge1 = positions[ind0] - positions[ind2];
		edge2 = positions[ind2] - positions[ind1];

		duv1 = texCoords[ind0] - texCoords[ind2];
		duv2 = texCoords[ind2] - texCoords[ind1];

		float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);

		//Find tangent using both tex coord edges and position edges
		tangent.x = (duv1.y * edge1.x - duv2.y * edge2.x) * f;
		tangent.y = (duv1.y * edge1.y - duv2.y * edge2.y) * f;
		tangent.z = (duv1.y * edge1.z - duv2.y * edge2.z) * f;

		tangent.Normalize();

		return tangent;
	}



	void Geometry::GenBox(SVec3 dims)
	{
		std::vector<DirectX::VertexPositionNormalTexture> verts;
		std::vector<uint16_t> inds;

		DirectX::GeometricPrimitive::CreateBox(verts, inds, DirectX::XMFLOAT3(dims.x, dims.y, dims.z), false, false);

		positions.reserve(verts.size());
		texCoords.reserve(verts.size());
		normals.reserve(verts.size());
		tangents.reserve(verts.size());

		for (auto v : verts)
		{
			positions.push_back(v.position);
			texCoords.push_back(v.textureCoordinate);
			normals.push_back(v.normal);
		}

		indices.reserve(inds.size());
		for (auto i : inds)
			indices.push_back(i);

		//go through faces 3 at a time
		std::map<int, SVec3> indexTangentMap;
		for (int i = 0; i < indices.size(); i += 3)
		{
			SVec3 curFaceTangent = calculateTangent(positions, texCoords, indices[i], indices[i + 1], indices[i + 2]);
			indexTangentMap[indices[i]]		+= curFaceTangent;
			indexTangentMap[indices[i + 1]] += curFaceTangent;
			indexTangentMap[indices[i + 2]] += curFaceTangent;
		}

		//tangents are now added up, should be normalized
		for (auto& tango : indexTangentMap)
			tango.second.Normalize();

		tangents.resize(positions.size());
		for (int i = 0; i < tangents.size(); ++i)
			tangents[i] = indexTangentMap[i];
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
			indices.push_back(indices[i] + oldVertCount);
		
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
	


	void Geometry::GenTube(float radius, float height, UINT subdivsRadial, UINT rows, float minRadiusPerc)
	{
		positions.reserve(rows * subdivsRadial);
		indices.reserve(subdivsRadial * 2 * 3 * (rows - 1));	//number of faces per column, times 2 for two subrows, times face rows, times 3
		texCoords.reserve(positions.size());

		float subdivHeight = height / float(rows - 1);

		float radialSegment = 1.f / float(subdivsRadial);
		float invRows = 1.f / float(rows);

		//create a ring
		float angle = 0.f;
		float dAngle = 2. * PI / float(subdivsRadial);

		float minRadius = radius * minRadiusPerc;

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
			float adjRadius = Math::remap(i, 0.f, rows - 1.f, radius, minRadius);

			for (UINT j = 0; j < subdivsRadial; ++j)
			{
				normals.emplace_back(cosines[j], 0.f, sines[j]);
				positions.emplace_back(normals.back().x * adjRadius, i * subdivHeight, normals.back().z * adjRadius);
				texCoords.emplace_back(j * radialSegment, i * invRows);
				tangents.emplace_back(-sines[j], 0.f, cosines[j]);
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
		normals.reserve(verts.size());
		texCoords.reserve(verts.size());
		tangents.reserve(verts.size());

		for (auto v : verts)
		{
			positions.emplace_back(v.position);
			normals.emplace_back(v.normal);
			texCoords.emplace_back(v.textureCoordinate);
			tangents.emplace_back(normals.back().Cross(SVec3(v.textureCoordinate.x, 0.f, v.textureCoordinate.y)));	//not correct at all but it does not matter! they don't show
		}
			
		indices.reserve(inds.size());
		for (auto i : inds)
			indices.emplace_back(i);
	}



	void Geometry::GenHexaprism(float radius, float thiccness)
	{
		float height = thiccness * .5f;
		SMatrix rot = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * .333333);
		SVec3 point(0, height, radius);

		float invRadius = 1.f / radius;

		positions.reserve(14);
		normals.reserve(14);
		texCoords.reserve(14);
		tangents.reserve(14);

		indices.reserve(72);	//6 faces up, 6 faces down, 12 faces around -> 24 faces, 3 indices each, 72 faces

		//first vertex
		positions.emplace_back(0, height, 0);
		normals.emplace_back(0, 1, 0);
		texCoords.emplace_back(0, 0);
		tangents.emplace_back(0, 0, 1);

		for (unsigned int i = 0; i < 6; ++i)
		{
			Math::RotateVecByMat(point, rot);
			positions.emplace_back(point);
			normals.emplace_back(Math::getNormalizedVec3(point));
			texCoords.emplace_back(point.x * invRadius, point.z * invRadius);
			//tangents.emplace_back(normals.back().Cross(SVec3(-normals.back().x, normals.back().y, -normals.back().z)));	//cross normal with flipped xz normal pointing inwards
			tangents.emplace_back(point.x * invRadius, 0,  point.z * invRadius);
			indices.insert(indices.end(), { 0, i + 1, i + 2 });
		}
		indices.back() = 1;	//close the hexagon

		for (unsigned int i = 1; i < 6; ++i)
		{
			indices.insert(indices.end(), { i + 8, i + 1, i + 7 });
			indices.insert(indices.end(), { i + 7, i + 1, i     });	//side down
		}

		indices.insert(indices.end(), { 1 + 7, 1    , 6 + 7 });
		indices.insert(indices.end(), { 6 + 7, 1    , 6     });

		//8th vertex
		positions.emplace_back(0, -height, 0);
		normals.emplace_back(0, -1, 0);
		texCoords.emplace_back(0, 0);
		tangents.emplace_back(0, 0, -1);

		for (unsigned int i = 1; i < 7; ++i)
		{
			SVec3 newPoint(positions[i].x, -positions[i].y, positions[i].z);
			positions.emplace_back(newPoint);
			normals.emplace_back(Math::getNormalizedVec3(newPoint));
			texCoords.emplace_back(newPoint.x * invRadius, newPoint.z * invRadius);
			tangents.emplace_back(normals.back().Cross(SVec3(-normals.back().x, normals.back().y, -normals.back().z)));
			indices.insert(indices.end(), { 7, i + 8, i + 7 });
		}
		indices[indices.size() - 2] = 8;	//close the hexagon
	}



	std::vector<Geometry> Geometry::GenHexGrid(float radius, float thiccness, UINT numHexes)
	{
		std::vector<Geometry> result;

		int initRingSize = 6, currentRingSize = 0;
		float hexHeight = radius * sqrt(3.f) * .5f;
		float hexCentDist = 2.f * hexHeight;

		SMatrix halfRot = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * .333333 * .5f);
		SMatrix fullRot = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * .333333);

		SVec3 offset = SVec3(0, 0, hexCentDist);
		offset = SVec3::Transform(offset, halfRot);	//this is the upper right hex position

		Geometry g, temp;
		g.GenHexaprism(radius, thiccness);
		temp = g;
		result.push_back(g);	//middle hex

		for (UINT i = 0; i < g.positions.size(); ++i)
			temp.positions[i] = g.positions[i] + offset;	//offset hex

		result.push_back(temp);

		temp.positions = g.positions;

		for (int i = 0; i < 5; ++i)
		{
			offset = SVec3::Transform(offset, fullRot);

			for (UINT i = 0; i < g.positions.size(); ++i)
				temp.positions[i] = g.positions[i] + offset;

			result.push_back(temp);

			temp.positions = g.positions;
		}

		return result;
	}



	void Geometry::Clear()
	{
		positions.clear();
		normals.clear();
		indices.clear();
		tangents.clear();
	}
}