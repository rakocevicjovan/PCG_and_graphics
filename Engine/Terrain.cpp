#include "pch.h"
#include "Terrain.h"
#include "Chaos.h"
#include "Perlin.h"
#include "Phong.h"
#include "Camera.h"


namespace Procedural 
{
	Terrain::Terrain(uint32_t rows, uint32_t columns, SVec3 scale, SVec3 offset) 
		: _numRows(rows), _numColumns(columns), _scale(scale), _texCoordScale(1.f, 1.f)
	{
		_vertices.reserve(rows * columns);
		
		for (int z = 0; z < rows; ++z) 
		{
			for(int x = 0; x < columns; ++x)
			{
				_vertices.emplace_back(SVec3(x * scale.x, 0, z * scale.y));
			}
		}

		CalculateTexCoords();

		CalculateNormals();

		_offset = offset;
	}


	// TBD
	Terrain::~Terrain()
	{
	}
	



	void Terrain::setTextureData(ID3D11Device* device, float xRepeat, float zRepeat, std::vector<std::string> textureNames)
	{
		_texCoordScale = SVec2(xRepeat, zRepeat);

		for (auto tn : textureNames)
		{
			textures.push_back(Texture(device, tn));
		}
	}



	void Terrain::Tumble(float chance, float displacement) 
	{
		Chaos chaos;

		std::vector<float> wat;
		chaos.fillVector(wat, _vertices.size());

		for (int i = 0; i < _vertices.size(); i++)
			if (wat[i] < chance)
				_vertices[i].pos.y += displacement * _scale.y;
	}



	float Terrain::sampleDiamond(int i, int j, int reach)
	{
		std::vector<float> heights;	

		if (i - reach >= 0)
			heights.push_back(_vertices[(i - reach) * _numColumns + j].pos.y);

		if (j - reach >= 0)
			heights.push_back(_vertices[i * _numColumns + j - reach].pos.y);

		if (j + reach < _numColumns)
			heights.push_back(_vertices[i * _numColumns + j + reach].pos.y);

		if (i + reach < _numRows)
			heights.push_back(_vertices[(i + reach) * _numColumns + j].pos.y);
		
		float result = 0.f;
		for (float h : heights)
			result += h;

		result /= (float)heights.size();

		return result;
	}




	void Terrain::GenWithDS(SVec4 corners, uint32_t steps, float decay, float randomMax) 
	{
		_numRows = _numColumns = pow(2, steps) + 1;
		uint32_t stepSize = _numRows - 1;

		_vertices.clear();
		_vertices.resize(_numRows * _numRows);

		for (int z = 0; z < _numRows; z++) 
			for(int x = 0; x < _numRows; x++)
				_vertices[z * _numRows + x].pos = SVec3(x * _scale.x, 0.f, z * _scale.z);
		

		//assign the corner values
		_vertices.front().pos.y = corners.x;
		_vertices[_vertices.size() - _numRows].pos.y = corners.y;
		_vertices.back().pos.y = corners.z;
		_vertices[stepSize].pos.y = corners.w;

		Chaos c(0, randomMax);

		//run the loop
		while (stepSize > 1)
		{
			int halfStep = stepSize / 2;	//half step, in this case 2 then 1

			//square
			for (int z = 0; z < _numRows - 1; z += stepSize)
			{
				for (int x = 0; x < _numRows - 1; x += stepSize)
				{
					int midRow = z + halfStep, midColumn = x + halfStep;
					int midVertIndex = midRow * _numRows + midColumn;

					float finHeight =
							_vertices[z * _numRows + x].pos.y +
							_vertices[z * _numRows + x + stepSize].pos.y +
							_vertices[(z + stepSize) * _numRows + x].pos.y +
							_vertices[(z + stepSize) * _numRows + x + stepSize].pos.y;
					
					finHeight *= 0.25f;
					finHeight += (c.rollTheDice() * 2.f - randomMax);

					_vertices[midVertIndex].pos.y = finHeight * _scale.y;
				}
			}

			//diamond
			for (int x = 0; x < _numRows - 1; x += stepSize)
			{
				for (int z = 0; z < _numRows - 1; z += stepSize)
				{
					float ro = c.rollTheDice() * 2.f - randomMax;

					_vertices[x * _numRows +	z + halfStep].pos.y = sampleDiamond(x, z + halfStep, halfStep) + ro;
					_vertices[(x + halfStep) *	_numRows + z].pos.y = sampleDiamond(x + halfStep, z, halfStep) + ro;
					_vertices[(x + halfStep) *	_numRows + z + stepSize].pos.y = sampleDiamond(x + halfStep, z + stepSize, halfStep) + ro;
					_vertices[(x + stepSize) *	_numRows + z + halfStep].pos.y = sampleDiamond(x + stepSize, z + halfStep, halfStep) + ro;
				}
			}

			//prepare for next iteration
			stepSize = stepSize / 2;
			randomMax *= decay;
			c.setRange(0, randomMax);
		}
	}



	void Terrain::CellularAutomata(float initialDistribtuion, uint32_t steps)
	{
		Chaos chaos(0.f, 1.f);

		std::vector<float> randoms;
		chaos.fillVector(randoms, _vertices.size());

		std::vector<bool> cells(_vertices.size(), false);

		for (int i = 0; i < randoms.size(); i++)
		{
			if (randoms[i] < initialDistribtuion)
				cells[i] = true;
		}

		std::vector<bool> cellsNext(_vertices.size(), false);

		//do the CA stuff
		for (uint32_t step = 0; step < steps; ++step)
		{
			for (uint32_t z = 0; z < _numRows; ++z)
			{
				for (uint32_t x = 0; x < _numColumns; ++x)
				{
					uint32_t index = z * _numColumns + x;

					//wrap the grid
					uint32_t topRow = (z == 0) ? _numRows - 1 : z - 1;
					uint32_t bottomRow = (z == _numRows - 1) ? 0 : z + 1;
					uint32_t leftColumn = (x == 0) ? _numColumns - 1 : x - 1;
					uint32_t rightColumn = (x == _numColumns - 1) ? 0 : x + 1;
					
					uint32_t count = 0;

					//check the 8 surrounding cells (Moor configuration)
					if(cells[topRow * _numColumns + leftColumn]) count++;
					if(cells[topRow * _numColumns + x]) count++;
					if(cells[topRow * _numColumns + rightColumn]) count++;
					
					if(cells[z * _numColumns + leftColumn]) count++;
					if(cells[z * _numColumns + rightColumn]) count++;

					if (cells[bottomRow * _numColumns + leftColumn]) count++;
					if (cells[bottomRow * _numColumns + x]) count++;
					if (cells[bottomRow * _numColumns + rightColumn]) count++;

					if (count >= 5)	//|| count <= 1
						cellsNext[index] = true;
					else
						cellsNext[index] = false;
					
				}
			}
			cells = cellsNext;
		}

		//write to the vertices based on the cells array
		for(int i = 0; i < _vertices.size(); ++i)
		{
			if (cells[i])
				_vertices[i].pos.y = _scale.y;
		}
	}



	void Terrain::GenFromTexture(uint32_t width, uint32_t height, const std::vector<float>& data) 
	{
		_numColumns = width;
		_numRows = height;

		_vertices.clear();
		_vertices.reserve(width * height);

		for (int z = 0; z < _numRows; ++z)
		{
			for(int x = 0; x < _numColumns; ++x)
			{
				_vertices.push_back(Vert3D(SVec3(x, data[z * _numColumns + x], z) * _scale));
			}
		}
	}



	void Terrain::CalculateNormals()
	{
		faces.resize(_numRows - 1);

		for (auto fRow : faces) 
			fRow.reserve((_numColumns - 1) * 2);

		//creating a double vector of faces and calculating the normals for each face
		for (int row = 0; row < _numRows - 1; ++row)
		{
			for (int column = 0; column < _numColumns - 1; ++column)
			{
				int tli = (row + 1) * _numColumns + column;
				int tri = tli + 1;
				int bli = row * _numColumns + column;
				int bri = bli + 1;

				Vert3D& topLeft = _vertices[tli];
				Vert3D& topRight = _vertices[tri];
				Vert3D& bottomLeft = _vertices[bli];
				Vert3D& bottomRight = _vertices[bri];

				//top left face
				SVec3 ab = topLeft.pos - topRight.pos;
				SVec3 ac = bottomLeft.pos - topRight.pos;
				SVec3 normal = ac.Cross(ab);

				faces[row].emplace_back(tli, tri, bli, normal, calculateTangent(_vertices, tli, tri, bli));

				//bottom right face
				ab = topRight.pos - bottomRight.pos;
				ac = bottomLeft.pos - bottomRight.pos;
				normal = ac.Cross(ab);

				faces[row].emplace_back(bli, tri, bri, normal, calculateTangent(_vertices, bli, tri, bri));
			}
		}


		// Calculate vertex normals from containing faces
		for (int i = 0; i < _numRows; i++)
		{
			std::vector<TangentTriface>* pRow{ nullptr };
			std::vector<TangentTriface>* nRow{ nullptr };

			if (i == 0)
			{
				nRow = &faces[i];
			}
			else if (i == _numRows - 1)
			{
				pRow = &faces[i - 1];
			}
			else
			{
				pRow = &faces[i - 1];
				nRow = &faces[i];
			}

			for (int j = 0; j < _numColumns; ++j) 
			{
				int index = i * _numColumns + j;
				uint32_t facesFound = 0;
				
				SVec3 normal;
				SVec3 tangent;

				if (pRow)
				{
					for (const auto& face : *pRow)
					{
						facesFound += addToFace(index, face, normal, tangent);
						if (facesFound == 6)
							break;
					}
				}

				if (nRow)
				{
					for (const auto& face : *nRow)
					{
						facesFound += addToFace(index, face, normal, tangent);
						if (facesFound == 6)
							break;
					}
				}

				if (fabs(normal.LengthSquared()) > 0.000001f)
					normal.Normalize();

				if (fabs(tangent.LengthSquared()) > 0.000001f)
					tangent.Normalize();

				_vertices[index].normal = normal;
				_vertices[index].tangent = tangent;
			}
		}


		_indices.clear();
		_indices.reserve(faces.size() * faces.size() * 6);	//square grid of faces, 2 faces per square, 3 indices per face

		for (auto row : faces)
		{
			for (auto face : row)
			{
				_indices.push_back((uint32_t)face.x);
				_indices.push_back((uint32_t)face.y);
				_indices.push_back((uint32_t)face.z);
			}
		}
	}



	void Terrain::CalculateTexCoords()
	{
		float invXScale = _texCoordScale.x / (_scale.x * _numColumns);
		float invZScale = _texCoordScale.y / (_scale.z * _numRows);

		for (auto& v : _vertices)
			v.texCoords = SVec2(v.pos.x * invXScale, v.pos.z * invZScale);
	}



	bool Terrain::SetUp(ID3D11Device* device) 
	{
		_vertexBuffer = VBuffer(device, _vertices.data(), _vertices.size() * sizeof(Vert3D));

		_indexBuffer = IBuffer(device, _indices);

		return true;
	}



	void Terrain::Draw(ID3D11DeviceContext* dc, Phong& s, const Camera& cam, const PointLight& pointLight, float deltaTime)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		WMBuffer* dataPtr;
		LightBuffer* dataPtr2;

		SMatrix mT = SMatrix::CreateTranslation(_offset).Transpose();
		SMatrix vT = cam.GetViewMatrix().Transpose();
		SMatrix pT = cam.GetProjectionMatrix().Transpose();

		if (FAILED(dc->Map(s._matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			return;
		dataPtr = (WMBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
		dataPtr->world = mT;
		dc->Unmap(s._matrixBuffer, 0);
		dc->VSSetConstantBuffers(0, 1, &s._matrixBuffer);


		if (FAILED(dc->Map(s._lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			return;
		dataPtr2 = (LightBuffer*)mappedResource.pData;
		dataPtr2->alc = pointLight.alc;
		dataPtr2->ali = pointLight.ali;
		dataPtr2->dlc = pointLight.dlc;
		dataPtr2->dli = pointLight.dli;
		dataPtr2->slc = pointLight.slc;
		dataPtr2->sli = pointLight.sli;
		dataPtr2->pos = pointLight.pos;
		dc->Unmap(s._lightBuffer, 0);
		dc->PSSetConstantBuffers(0, 1, &s._lightBuffer);

		uint32_t stride = sizeof(Vert3D);
		uint32_t offset = 0;

		dc->VSSetShader(s._vertexShader, NULL, 0);
		dc->PSSetShader(s._pixelShader, NULL, 0);

		dc->PSSetSamplers(0, 1, &s._sampleState);

		for (int i = 0; i < textures.size(); ++i)
			dc->PSSetShaderResources(i, 1, &(textures[i]._arraySrv));
			
		_indexBuffer.bind(dc);
		_vertexBuffer.bind(dc);

		dc->PSSetSamplers(0, 1, &s._sampleState);
		dc->IASetInputLayout(s._layout);
		dc->DrawIndexed(_indices.size(), 0, 0);

#ifdef _DEBUG
		dc->PSSetShaderResources(0, 1, &(unbinder[0]));	// Prevent leaking textures when debugging
#endif
	}



	void Terrain::Fault(const SRay& line, float displacement) 
	{
		float adjX = line.position.x;
		float adjZ = line.position.z;

		for (int i = 0; i < _vertices.size(); ++i)
			if (line.direction.z * (_vertices[i].pos.x - adjX) - line.direction.x * (_vertices[i].pos.z - adjZ) > 0)
				_vertices[i].pos.y += displacement;
	}



	void Terrain::NoisyFault(const SRay& line, float vertDp, float horiDp, float perlinZoom)
	{
		float rox = line.position.x;
		float roz = line.position.z;

		//SVec3 adjFaultPos = SVec3(Math::smoothstep(0, 1, line.position.x), 0, Math::smoothstep(0, 1, line.position.z));

		//to put the vertices in the 0-1 range
		float inverseWidth = 1.f / ((float)_numColumns * _scale.x);
		float inverseDepth = 1.f / ((float)_numRows    * _scale.z);
		
		Perlin p;

		for (int i = 0; i < _vertices.size(); ++i)
		{
			Vert3D cv = _vertices[i];
			SVec2 input = SVec2(cv.pos.x * inverseWidth, cv.pos.z * inverseDepth);
			float p2dVal = p.perlin2d(input * perlinZoom) * horiDp;
			
			if (line.direction.z * (cv.pos.x - rox) - line.direction.x * (cv.pos.z - roz) > p2dVal)
				_vertices[i].pos.y += vertDp;
		}
	}



	//keep decay under 1 for reasonable results? Still, something fun could happen otherwise... this is ugly anyways
	void Terrain::TerraSlash(const SRay& line, float displacement, uint32_t steps, float decay)
	{
		Chaos c;

		Fault(line, displacement);

		for (int i = 1; i < steps; ++i)
		{
			c.setRange(0, _numColumns);
			float newLineX = c.rollTheDice();
			c.setRange(0, _numRows);
			float newLineZ = c.rollTheDice();
			SVec3 randomAxis(newLineX, 0.f, newLineZ);

			c.setRange(0, PI);
			SVec3 randomDir = SVec3::Transform(SVec3::Right, SMatrix::CreateFromAxisAngle(SVec3::Up, c.rollTheDice()));
			displacement *= decay;

			Fault(SRay(randomAxis, randomDir), displacement);
		}
	}



	void Terrain::CircleOfScorn(const SVec2& center, float radius, float angle, float displacement, uint32_t steps, float initAngle)
	{
		float curAngle = initAngle;

		for (int i = 0; i < steps; ++i)
		{
			float cosAng = cos(curAngle);
			float sinAng = sin(curAngle);

			SVec2 dir(-sinAng, cosAng);

			SVec2 curPoint = center + dir * radius;

			SVec3 curPoint3D(curPoint.x, 0.f, curPoint.y);
			SVec3 curTangent3D(-dir.y, 0.f, dir.x);

			Fault(SRay(curPoint3D, curTangent3D), displacement);
			//NoisyFault(SRay(curPoint3D, curTangent3D), displacement, 200.f);
			curAngle += angle;
		}
	}



	void Terrain::Mesa(const SVec2& center, float radius, float bandWidth, float height)
	{
		float inner = radius * radius;
		float outer = pow((radius + bandWidth), 2);

		for (int i = 0; i < _vertices.size(); ++i)
		{
			float sqDistToCenter = SVec2::DistanceSquared(SVec2(_vertices[i].pos.x, _vertices[i].pos.z), center);
			_vertices[i].pos.y += Math::smoothstep(outer, inner, sqDistToCenter) * height;
		}
	}

	

	//y[i] = k * y[i-j] + (1-k) * x[i], where k is a filtering constant (erosion coefficient) such that 0 <= k <= 1
	//apply this FIR function to rows and columns individually, in both directions
	void Terrain::Smooth(uint32_t steps) 
	{
		for (uint32_t i = 0; i < steps; ++i)
		{
			std::vector<float> smoothed;
			smoothed.reserve(_vertices.size());

			for (auto z = 0; z < _numRows; ++z)
			{
				for (int x = 0; x < _numColumns; ++x)
				{
					int pX = x == 0					? x : x - 1;
					int nX = x == _numColumns - 1	? x : x + 1;

					smoothed.push_back((
						_vertices[z * _numColumns + pX].pos.y +
						_vertices[z * _numColumns + x].pos.y +
						_vertices[z * _numColumns + nX].pos.y)
						* .33333f);
				}
			}

			for (int z = 0; z < _numRows; ++z)
			{
				int pZ = z == 0				? z : z - 1;
				int nZ = z == _numRows - 1	? z : z + 1;

				for (int x = 0; x < _numColumns; ++x)
				{
					int thisSmoothed = z * _numColumns + x;
					float newHeight =
						_vertices[pZ * _numColumns + x].pos.y +
						_vertices[thisSmoothed].pos.y +
						_vertices[nZ * _numColumns + x].pos.y;
				
					smoothed[thisSmoothed] += (newHeight * .333333f);
					smoothed[thisSmoothed] *= .500000f;
				}
			}

			for (int i = 0; i < _vertices.size(); ++i)
				_vertices[i].pos.y = smoothed[i];
		}
	}



	std::vector<SVec2> Terrain::getHorizontalPositions() 
	{
		std::vector<SVec2> result;
		result.reserve(_vertices.size());
		
		for (int i = 0; i < _vertices.size(); i++)
			result.emplace_back(_vertices[i].pos.x, _vertices[i].pos.z);

		return result;
	}



	float getHeightByBarrycentric(const SVec3& p1, const SVec3& p2, const SVec3& p3, const SVec2& pos)
	{
		float detInverse  = 1.f / ((p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z));
		float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) * detInverse;
		float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) * detInverse;
		float l3 = 1.0f - l1 - l2;
		return l1 * p1.y + l2 * p2.y + l3 * p3.y;
	}



	float Terrain::getHeightAtPosition(const SVec3& playerPos)
	{
		float terX = playerPos.x - _offset.x;
		float terZ = playerPos.z - _offset.z;

		int gridX = (int)floorf(terX / _scale.x);
		int gridZ = (int)floorf(terZ / _scale.z);

		if (gridX >= _numColumns - 1 || gridZ >= _numRows - 1 || gridX < 0 || gridZ < 0)
			return _offset.y;

		// To barycentric
		float xCoord = fmodf(terX, _scale.x) / _scale.x;
		float zCoord = fmodf(terZ, _scale.z) / _scale.z;

		int tli = (gridZ + 1) * _numColumns + gridX;
		int tri = tli + 1;
		int bli = gridZ * _numColumns + gridX;
		int bri = bli + 1;

		float finalHeight = 0.f;
		float trh = _vertices[tri].pos.y;
		float blh = _vertices[bli].pos.y;

		float x = 0.f, y = 0.f, z = 0.f;

		if (xCoord < zCoord)	//x = 1 - z is the center line for how I subdivide quads into two triangle faces
		{//top left triangle
			float tlh = _vertices[tli].pos.y;
			finalHeight = getHeightByBarrycentric(SVec3(0, tlh, 1), SVec3(1, trh, 1), SVec3(0, blh, 0), SVec2(xCoord, zCoord));
		}
		else
		{//bottom right triangle
			float brh = _vertices[bri].pos.y;
			finalHeight = getHeightByBarrycentric(SVec3(0, blh, 0), SVec3(1, trh, 1), SVec3(1, brh, 0), SVec2(xCoord, zCoord));
		}

		return finalHeight + _offset.y;
	}



	SVec3 Terrain::calculateTangent(const std::vector<Vert3D>& vertices, UINT i0, UINT i1, UINT i2)
	{
		SVec3 tangent;
		SVec3 edge1, edge2;
		SVec2 duv1, duv2;

		//Find first texture coordinate edge 2d vector
		Vert3D v0 = vertices[i0];
		Vert3D v1 = vertices[i1];
		Vert3D v2 = vertices[i2];

		edge1 = v0.pos - v2.pos;
		edge2 = v2.pos - v1.pos;

		duv1 = v0.texCoords - v2.texCoords;
		duv2 = v2.texCoords - v1.texCoords;

		float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);

		//Find tangent using both tex coord edges and position edges
		tangent.x = (duv1.y * edge1.x - duv2.y * edge2.x) * f;
		tangent.y = (duv1.y * edge1.y - duv2.y * edge2.y) * f;
		tangent.z = (duv1.y * edge1.z - duv2.y * edge2.z) * f;

		tangent.Normalize();

		return tangent;
	}
}