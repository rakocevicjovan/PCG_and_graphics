#include "Terrain.h"
#include "Chaos.h"
#include "Perlin.h"

namespace Procedural 
{

	Terrain::Terrain(unsigned int rows, unsigned int columns, SVec3 scales) 
		: _numRows(rows), _numColumns(columns), xScale(scales.x), yScale(scales.y), zScale(scales.z)
	{
		vertices.clear();
		vertices.reserve(rows * columns);
		Vert3D v;
		for (int z = 0; z < rows; ++z) 
		{
			for(int x = 0; x < columns; ++x)
			{
				v.pos = SVec3(x * xScale, 0, z * zScale);
				vertices.push_back(v);
			}
		}
	}

	Terrain::~Terrain()
	{
	}



	void Terrain::setScales(float x, float y, float z) 
	{
		xScale = x;
		yScale = y;
		zScale = z;
	}



	void Terrain::setTextureData(ID3D11Device* device, float xRepeat, float zRepeat, std::vector<std::string> textureNames)
	{
		tcxr = xRepeat;
		tczr = zRepeat;

		for (auto tn : textureNames)
		{
			textures.push_back(Texture(device, tn));
		}
	}



	void Terrain::Tumble(float chance) 
	{
		Chaos chaos;

		std::vector<float> wat;
		chaos.fillVector(wat, vertices.size());

		for (int i = 0; i < vertices.size(); i++)
			if (wat[i] < chance)
				vertices[i].pos.y += yScale;
	}



	inline unsigned int Terrain::wr(int row) { return row < 0 ? _numRows + row : row % _numRows; }
	inline unsigned int Terrain::wc(int col) { return col < 0 ? _numColumns + col : col % _numColumns; }
	float Terrain::sampleDiamond(int i, int j, int reach)
	{
		std::vector<float> heights;	

		if (i - reach >= 0)
			heights.push_back(vertices[(i - reach) * _numColumns + j].pos.y);

		if (j - reach >= 0)
			heights.push_back(vertices[i * _numColumns + j - reach].pos.y);

		if (j + reach < _numColumns)
			heights.push_back(vertices[i * _numColumns + j + reach].pos.y);

		if (i + reach < _numRows)
			heights.push_back(vertices[(i + reach) * _numColumns + j].pos.y);
		
		float result = 0.f;
		for (float h : heights)
			result += h;

		result /= (float)heights.size();

		return result;
	}

	void Terrain::GenWithDS(SVec4 corners, unsigned int steps, float decay, float randomMax) 
	{
		_numRows = _numColumns = pow(2, steps) + 1;
		unsigned int stepSize = _numRows - 1;

		vertices.clear();
		vertices.resize(_numRows * _numRows);

		for (int z = 0; z < _numRows; z++) 
			for(int x = 0; x < _numRows; x++)
				vertices[z * _numRows + x].pos = SVec3(x * xScale, 0.f, z * zScale);
		

		//assign the corner values
		vertices.front().pos.y = corners.x;
		vertices[vertices.size() - _numRows].pos.y = corners.y;
		vertices.back().pos.y = corners.z;
		vertices[stepSize].pos.y = corners.w;

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
							vertices[z * _numRows + x].pos.y +
							vertices[z * _numRows + x + stepSize].pos.y +
							vertices[(z + stepSize) * _numRows + x].pos.y +
							vertices[(z + stepSize) * _numRows + x + stepSize].pos.y;
					
					finHeight *= 0.25f;
					finHeight += (c.rollTheDice() * 2.f - randomMax);

					vertices[midVertIndex].pos.y = finHeight * yScale;
				}
			}


			//diamond
			for (int x = 0; x < _numRows - 1; x += stepSize)
			{
				for (int z = 0; z < _numRows - 1; z += stepSize)
				{
					float ro = c.rollTheDice() * 2.f - randomMax;

					vertices[x * _numRows +	z + halfStep].pos.y = sampleDiamond(x, z + halfStep, halfStep) + ro;
					vertices[(x + halfStep) *	_numRows + z].pos.y = sampleDiamond(x + halfStep, z, halfStep) + ro;
					vertices[(x + halfStep) *	_numRows + z + stepSize].pos.y = sampleDiamond(x + halfStep, z + stepSize, halfStep) + ro;
					vertices[(x + stepSize) *	_numRows + z + halfStep].pos.y = sampleDiamond(x + stepSize, z + halfStep, halfStep) + ro;
				}
			}


			//prepare for next iteration
			stepSize = stepSize / 2;
			randomMax *= decay;
			c.setRange(0, randomMax);
		}
	}



	void Terrain::CellularAutomata(float initialDistribtuion, unsigned int steps)
	{
		Chaos chaos(0.f, 1.f);

		std::vector<float> randoms;
		chaos.fillVector(randoms, vertices.size());

		std::vector<bool> cells(vertices.size(), false);

		for (int i = 0; i < randoms.size(); i++)
		{
			if (randoms[i] < initialDistribtuion)
				cells[i] = true;
		}

		std::vector<bool> cellsNext(vertices.size(), false);

		//do the CA stuff
		for (unsigned int step = 0; step < steps; ++step)
		{
			for (unsigned int z = 0; z < _numRows; ++z)
			{
				for (unsigned int x = 0; x < _numColumns; ++x)
				{
					unsigned int index = z * _numColumns + x;

					//wrap the grid
					unsigned int topRow = (z == 0) ? _numRows - 1 : z - 1;
					unsigned int bottomRow = (z == _numRows - 1) ? 0 : z + 1;
					unsigned int leftColumn = (x == 0) ? _numColumns - 1 : x - 1;
					unsigned int rightColumn = (x == _numColumns - 1) ? 0 : x + 1;
					
					unsigned int count = 0;

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
		for(int i = 0; i < vertices.size(); ++i)
		{
			if (cells[i])
				vertices[i].pos.y = yScale;
		}
	}



	void Terrain::GenFromTexture(unsigned int width, unsigned int height, const std::vector<float>& data) 
	{
		_numColumns = width;
		_numRows = height;

		vertices.clear();
		vertices.reserve(width * height);

		for (int z = 0; z < _numRows; ++z)
		{
			for(int x = 0; x < _numColumns; ++x)
			{
				vertices.push_back(Vert3D(SVec3(x * xScale, data[z * _numColumns + x] * yScale, z * zScale)));
			}
		}
	}



	void Terrain::CalculateNormals()
	{
		std::vector<std::vector<std::pair<SVec3, SVec3>>> faces;
		faces.resize(_numRows - 1);
		for (auto fRow : faces) fRow.reserve((_numColumns - 1) * 2);

		//creating a double vector of faces and calculating the normals for each face
		for (int row = 0; row < _numRows - 1; ++row)
		{
			for (int column = 0; column < _numColumns - 1; ++column)
			{
				int tli = (row + 1) * _numColumns + column;
				int tri = tli + 1;
				int bli = row * _numColumns + column;
				int bri = bli + 1;

				Vert3D topLeft = vertices[tli];
				Vert3D topRight = vertices[tri];
				Vert3D bottomLeft = vertices[bli];
				Vert3D bottomRight = vertices[bri];

				//top left face
				SVec3 ab = topLeft.pos - topRight.pos;
				SVec3 ac = bottomLeft.pos - topRight.pos;
				SVec3 normal = ac.Cross(ab);

				faces[row].push_back(std::make_pair(SVec3(tli, tri, bli), normal));

				//bottom right face
				ab = topRight.pos - bottomRight.pos;
				ac = bottomLeft.pos - bottomRight.pos;
				normal = ac.Cross(ab);

				faces[row].push_back(std::make_pair(SVec3(bli, tri, bri), normal));
			}
		}


		//calculating vertex normals from containing faces
		for(int i = 0; i < _numRows; i++)
		{	
			std::vector<std::pair<SVec3, SVec3>> pRow;
			std::vector<std::pair<SVec3, SVec3>> nRow;

			if (i == 0)
			{
				nRow = faces[i];
			}
			else if (i == _numRows - 1)
			{
				pRow = faces[i - 1];
			}
			else
			{
				pRow = faces[i - 1];
				nRow = faces[i];
			}

			pRow.reserve(pRow.capacity() + nRow.capacity());
			pRow.insert(pRow.end(), nRow.begin(), nRow.end());

			for (int j = 0; j < _numColumns; j++) 
			{
				int index = i * _numColumns + j;
				unsigned int facesFound = 0;
				SVec3 currentNormal;

				for (auto face : pRow)
				{
					if (index == face.first.x || index == face.first.y || index == face.first.z)
					{
						currentNormal += face.second;
						facesFound++;
					}

					if (facesFound == 6)
						break;
				}

				if (fabs(currentNormal.LengthSquared()) > 0.000001f)
					currentNormal.Normalize();

				vertices[index].normal = currentNormal;
			}
		}


		indices.clear();
		indices.reserve(faces.size() * faces.size() * 6);	//square grid of faces, 2 faces per square, 3 indices per face

		for (auto row : faces)
		{
			for (auto face : row)
			{
				indices.push_back((unsigned int)face.first.x);
				indices.push_back((unsigned int)face.first.y);
				indices.push_back((unsigned int)face.first.z);
			}
		}

		//generate UVs? I could normalize the whole terrain naively but it would stretch on slopes... proper calculations could be very long @TODO
	}



	bool Terrain::SetUp(ID3D11Device* device) 
	{
		CalculateNormals();
		
		float invXScale = tcxr / (xScale * _numColumns), invZScale = tczr / (zScale * _numRows);
		for (auto& v : vertices)
		{
			v.texCoords = SVec2(v.pos.x * invXScale, v.pos.z * invZScale);
		}

		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT res;

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(Vert3D) * vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = vertices.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		res = device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer);
		if (FAILED(res))
			return false;

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned int) * indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer)))
			return false;

		return true;
	}



	void Terrain::Draw(ID3D11DeviceContext* dc, ShaderBase& s, const SMatrix& mt, const SMatrix& vt, const SMatrix& pt, const PointLight& pointLight, float deltaTime, SVec3 eyePos)
	{
		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		MatrixBuffer* dataPtr;
		LightBuffer* dataPtr2;
		VariableBuffer* dataPtr3;

		// Lock the constant matrix buffer so it can be written to.
		result = dc->Map(s._matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;

		dataPtr = (MatrixBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.

		SMatrix mT = mt.Transpose();
		SMatrix vT = vt.Transpose();
		SMatrix pT = pt.Transpose();

		// Copy the matrices into the constant buffer.
		dataPtr->world = mT;
		dataPtr->view = vT;
		dataPtr->projection = pT;

		// Unlock the constant buffer.
		dc->Unmap(s._matrixBuffer, 0);

		bufferNumber = 0;	// Set the position of the constant buffer in the vertex shader.
		dc->VSSetConstantBuffers(bufferNumber, 1, &s._matrixBuffer);	// Now set the constant buffer in the vertex shader with the updated values.
		//END MATRIX BUFFER



		result = dc->Map(s._variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;

		dataPtr3 = (VariableBuffer*)mappedResource.pData;

		dataPtr3->deltaTime = deltaTime;
		dataPtr3->padding = SVec3();

		dc->Unmap(s._variableBuffer, 0);

		bufferNumber = 1;
		dc->VSSetConstantBuffers(bufferNumber, 1, &s._variableBuffer);



		result = dc->Map(s._lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;


		dataPtr2 = (LightBuffer*)mappedResource.pData;
		dataPtr2->alc = pointLight.alc;
		dataPtr2->ali = pointLight.ali;
		dataPtr2->dlc = pointLight.dlc;
		dataPtr2->dli = pointLight.dli;
		dataPtr2->slc = pointLight.slc;
		dataPtr2->sli = pointLight.sli;
		dataPtr2->pos = pointLight.pos;
		dataPtr2->ePos = SVec4(eyePos.x, eyePos.y, eyePos.z, 1.0f);

		dc->Unmap(s._lightBuffer, 0);

		bufferNumber = 0;

		unsigned int stride = sizeof(Vert3D);
		unsigned int offset = 0;

		dc->VSSetShader(s._vertexShader, NULL, 0);
		dc->PSSetShader(s._pixelShader, NULL, 0);

		dc->PSSetConstantBuffers(bufferNumber, 1, &s._lightBuffer);
		dc->PSSetSamplers(0, 1, &s._sampleState);

		for (int i = 0; i < textures.size(); ++i)
		{
			dc->PSSetShaderResources(i, 1, &(textures[i].srv));
		}
			

		dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dc->PSSetSamplers(0, 1, &s._sampleState);
		dc->IASetInputLayout(s._layout);

		dc->DrawIndexed(indices.size(), 0, 0);

		dc->PSSetShaderResources(0, 1, &(unbinder[0]));
	}



	void Terrain::Fault(const SRay& line, float displacement) 
	{
		float adjX = line.position.x;
		float adjZ = line.position.z;

		for (int i = 0; i < vertices.size(); ++i)
			if (line.direction.z * (vertices[i].pos.x - adjX) - line.direction.x * (vertices[i].pos.z - adjZ) > 0)
				vertices[i].pos.y += displacement;
	}



	void Terrain::NoisyFault(const SRay& line, float vertDp, float horiDp)
	{
		float adjX = line.position.x;
		float adjZ = line.position.z;

		float inverseWidth = 1.f / ((float)_numColumns * xScale);
		float inverseDepth = 1.f / ((float)_numRows    * zScale);
		
		Perlin p;


		for (int i = 0; i < vertices.size(); ++i)
		{
			Vert3D cv = vertices[i];
			SVec2 input = SVec2(cv.pos.x * inverseWidth, cv.pos.z * inverseDepth);
			float p2dVal = p.perlin2d(input) * horiDp;
			
			if (line.direction.z * (cv.pos.x - adjX) - line.direction.x * (cv.pos.z - adjZ) > p2dVal)
				vertices[i].pos.y += vertDp;
		}
	}



	//keep decay under 1 for reasonable results? Still, something fun could happen otherwise... this is ugly anyways
	void Terrain::TerraSlash(const SRay& line, float displacement, unsigned int steps, float decay)
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



	void Terrain::CircleOfScorn(const SVec2& center, float radius, float angle, float displacement, unsigned int steps)
	{
		float curAngle = 0;

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

	
	//y[i] = k * y[i-j] + (1-k) * x[i], where k is a filtering constant (erosion coefficient) such that 0 <= k <= 1
	//apply this FIR function to rows and columns individually, in both directions
	void Terrain::Smooth(unsigned int steps) 
	{
		for (int i = 0; i < steps; ++i)
		{
		std::vector<float> smoothed;
		smoothed.reserve(vertices.size());

		for (int z = 0; z < _numRows; ++z)
		{
			for (int x = 0; x < _numColumns; ++x)
			{
				int pX = x == 0					? x : x - 1;
				int nX = x == _numColumns - 1	? x : x + 1;

				smoothed.push_back((
					vertices[z * _numColumns + pX].pos.y +
					vertices[z * _numColumns + x].pos.y +
					vertices[z * _numColumns + nX].pos.y)
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
					vertices[pZ * _numColumns + x].pos.y +
					vertices[thisSmoothed].pos.y +
					vertices[nZ * _numColumns + x].pos.y;
				
				smoothed[thisSmoothed] += (newHeight * .333333f);
				smoothed[thisSmoothed] *= .500000f;
			}
		}

		for (int i = 0; i < vertices.size(); ++i)
			vertices[i].pos.y = smoothed[i];
		}
	}



	std::vector<SVec2> Terrain::getHorizontalPositions() 
	{
		std::vector<SVec2> result;
		result.reserve(vertices.size());
		
		for (int i = 0; i < vertices.size(); i++)
			result.push_back(SVec2(vertices[i].pos.x, vertices[i].pos.z));

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

		int gridX = (int)floorf(terX / xScale);
		int gridZ = (int)floorf(terZ / zScale);

		if (gridX >= _numColumns - 1 || gridZ >= _numRows - 1 || gridX < 0 || gridZ < 0)
			return 5.0f;

		//to barycentric
		float xCoord = fmodf(terX, xScale) / xScale;
		float zCoord = fmodf(terZ, zScale) / zScale;

		int tli = (gridZ + 1) * _numColumns + gridX;
		int tri = tli + 1;
		int bli = gridZ * _numColumns + gridX;
		int bri = bli + 1;

		float finalHeight = 0.f;
		float trh = vertices[tri].pos.y;
		float blh = vertices[bli].pos.y;

		float x = 0.f, y = 0.f, z = 0.f;

		if (xCoord < zCoord)	//x = 1 - z is the center line for how I subdivide quads into two triangle faces
		{//top left triangle
			float tlh = vertices[tli].pos.y;
			finalHeight = getHeightByBarrycentric(SVec3(0, tlh, 1), SVec3(1, trh, 1), SVec3(0, blh, 0), SVec2(xCoord, zCoord));
		}
		else
		{//bottom right triangle
			float brh = vertices[bri].pos.y;
			finalHeight = getHeightByBarrycentric(SVec3(0, blh, 0), SVec3(1, trh, 1), SVec3(1, brh, 0), SVec2(xCoord, zCoord));
		}

		return finalHeight + 10.f;
	}
}