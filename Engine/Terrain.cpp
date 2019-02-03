#include "Terrain.h"
#include "Chaos.h"

namespace Procedural 
{

	Terrain::Terrain(unsigned int rows, unsigned int columns, SVec3 scales) 
		: _numRows(rows), _numColumns(columns), xScale(scales.x), yScale(scales.y), zScale(scales.z)
	{
		vertices.clear();
		vertices.reserve(rows * columns);
		Vert3D v;
		for (int i = 0; i < rows; i++) 
		{
			for(int j = 0; j < columns; j++)
			{
				v.pos = SVec3(j * xScale, 0, i * zScale);
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



	void Terrain::GenRandom(float chance) 
	{
		Chaos chaos;

		std::vector<float> wat;
		wat.resize(vertices.size());
		chaos.fillVector(wat);

		//for (auto c : cells)
		//	c.deadOrAlive = (chaos.rollTheDice(0, 1) < 0.45f);	//45% chance to get true, 55% to get false

		for (int i = 0; i < vertices.size(); i++)
		{
			if (wat[i] < chance) {
				vertices[i].pos.y = yScale;
			}
		}
	}



	inline unsigned int Terrain::wr(int row) { return row < 0 ? _numRows + row : row % _numRows; }

	inline unsigned int Terrain::wc(int col) { return col < 0 ? _numColumns + col : col % _numColumns; }

	float Terrain::sampleDiamond(int i, int j, int reach)
	{
		return
			vertices[wr(i - reach) * _numColumns + j].pos.y +	//top
			vertices[i * _numColumns + wc(j - reach)].pos.y +	//left
			vertices[i * _numColumns + wc(j + reach)].pos.y +	//right
			vertices[wr(i + reach) * _numColumns + j].pos.y		//bottom
			;
	}

	//decay should be kept under 1.0f
	void Terrain::GenWithDS(SVec4 corners, unsigned int steps, float decay, float randomMax) 
	{
		//assume steps = 2
		_numRows = _numColumns = pow(2, steps) + 1;		//5 by 5 grid
		unsigned int stepSize = _numRows - 1;			//stepSize is 4

		vertices.clear();
		vertices.resize(_numRows * _numColumns);	//25 vertices

		for (int i = 0; i < _numRows; i++)
		{ 
			for(int j = 0; j < _numColumns; j++)
			{
				vertices[i * _numColumns + j].pos = SVec3(j * xScale, 0.f, i * zScale);
			}
		}
		

		//assign the corner values
		vertices.front().pos.y = corners.x;							//vertex at 0
		vertices[vertices.size() - _numColumns].pos.y = corners.y;	//first vertex of last row (20 in 5x5)
		vertices.back().pos.y = corners.z;							//vertex at 25
		vertices[stepSize].pos.y = corners.w;						//vertex at 4

		Chaos c(0, randomMax);	//initialize chaos and set the limits of the distribution between 0 and randomMax

		//run the loop
		while (stepSize > 1)
		{
			int halfStep = stepSize >> 1;	//half step, in this case 2 then 1

			//square
			for (int i = 0; i < _numRows - 1; i += stepSize)
			{
				for (int j = 0; j < _numColumns - 1; j += stepSize)
				{
					int midRow = i + halfStep, midColumn = j + halfStep;
					int midVertIndex = midRow * _numColumns + midColumn;

					float finHeight =
							vertices[i * _numColumns + j].pos.y +
							vertices[i * _numColumns + j + stepSize].pos.y +
							vertices[(i + stepSize) * _numColumns + j].pos.y +
							vertices[(i + stepSize) * _numColumns + j + stepSize].pos.y;
					
					finHeight *= 0.25f;
					finHeight += (c.rollTheDice() * 2.f - randomMax);

					vertices[midVertIndex].pos.y = finHeight * yScale;
				}
			}


			//diamond
			for (int i = 0; i < _numRows - 1; i += stepSize)
			{
				for (int j = 0; j < _numColumns - 1; j += stepSize)
				{
					int midRow = i + halfStep, midColumn = j + halfStep;
					Vert3D mid = vertices[midRow * _numColumns + midColumn];

					float ro = c.rollTheDice() * 2.f - randomMax;

					//top
					vertices[i *				_numColumns +	j + halfStep	].pos.y = sampleDiamond(i, j + halfStep, halfStep) * 0.25f + ro;
					//left
					vertices[(i + halfStep) *	_numColumns +	j				].pos.y = sampleDiamond(i + halfStep, j, halfStep) * 0.25f + ro;
					//right
					vertices[(i + halfStep) *	_numColumns +	j + stepSize	].pos.y = sampleDiamond(i + halfStep, j + stepSize, halfStep) * 0.25f + ro;
					//bottom
					vertices[(i + stepSize) *	_numColumns +	j + halfStep	].pos.y = sampleDiamond(i + stepSize, j + halfStep, halfStep) * 0.25f + ro;
				}
			}



			/*
			for (int x = 0; x < _numRows; x += halfStep)
			{
				for (int y = (x + halfStep) % stepSize; y < _numColumns; y += stepSize)
				{
					float finHeight =
						vertices[((x - halfStep + _numRows - 1) % (_numRows - 1))	* _numColumns	+ y].pos.y + 
						vertices[((x + halfStep) % (_numRows - 1))					* _numColumns	+ y].pos.y + 
						vertices[(x * _numColumns)	+ ((y + halfStep) % (_numColumns - 1))].pos.y + 
						vertices[(x * _numColumns)	+ ((y - halfStep + _numColumns - 1) % (_numColumns - 1))].pos.y; 
					
					finHeight *= 0.25f;
					finHeight += (c.rollTheDice() * 2.f - randomMax);

					vertices[x * _numColumns + y].pos.y = finHeight * yScale;

					//if (x == 0)  
						//vertices[(_numRows - 1) * _numColumns + y].pos.y = finHeight * yScale;
					//if (y == 0)  
						//vertices[x * _numColumns + _numColumns - 1].pos.y = finHeight * yScale;
				}
			}
			*/

			//prepare for next iteration
			stepSize >>= 1;
			randomMax *= decay;
			c.setRange(0, randomMax);
		}

	}



	void Terrain::GenWithCA(float initialDistribtuion, unsigned int steps)
	{

		//give it a random seed
		Chaos chaos;

		std::vector<float> randoms;
		randoms.resize(vertices.size());
		chaos.fillVector(randoms);

		std::vector<bool> cells(vertices.size(), false);

		for (int i = 0; i < randoms.size(); i++)
		{
			if (randoms[i] < initialDistribtuion)
				cells[i] = true;
		}

		//instantiate the other array which will be used in the meantime 
		std::vector<bool> cellsNext(vertices.size(), false);

		//do the CA stuff
		for (unsigned int step = 0; step < steps; ++step)
		{
			for (unsigned int i = 0; i < _numRows; ++i)
			{
				for (unsigned int j = 0; j < _numColumns; ++j)
				{
					unsigned int index = i * _numColumns + j;

					//wrap the grid
					unsigned int topRow = (i == 0) ? _numRows - 1 : i - 1;
					unsigned int bottomRow = (i == _numRows - 1) ? 0 : i + 1;
					unsigned int leftColumn = (j == 0) ? _numColumns - 1 : j - 1;
					unsigned int rightColumn = (j == _numColumns - 1) ? 0 : j + 1;
					
					unsigned int count = 0;

					//check the 8 surrounding cells (Moor configuration)
					if(cells[topRow * _numColumns + leftColumn]) count++;
					if(cells[topRow * _numColumns + j]) count++;
					if(cells[topRow * _numColumns + rightColumn]) count++;
					
					if(cells[i * _numColumns + leftColumn]) count++;
					if(cells[i * _numColumns + rightColumn]) count++;

					if (cells[bottomRow * _numColumns + leftColumn]) count++;
					if (cells[bottomRow * _numColumns + j]) count++;
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
		vertices.resize(width * height);

		for (int i = 0; i < vertices.size(); ++i)
		{
			int row = i / width;
			int column = i % width;

			vertices[i].pos = SVec3(column * xScale, data[i] * yScale, row * zScale);
		}
	}



	void Terrain::CalculateNormals()
	{
		std::vector<std::vector<std::pair<SVec3, SVec3>>> faces;

		faces.resize(_numRows - 1);

		for (auto fRow : faces)
			fRow.reserve((_numColumns - 1) * 2);

		//creating a double vector of faces and calculating the normals for each face
		for (int row = 0; row < _numRows - 1; row++)
		{
			for (int column = 0; column < _numColumns - 1; column++)
			{
				int tli = row * _numColumns + column;
				int tri = row * _numColumns + column + 1;
				int bli = (row + 1) * _numColumns + column;
				int bri = (row + 1) * _numColumns + column + 1;

				Vert3D topLeft = vertices[tli];
				Vert3D topRight = vertices[tri];
				Vert3D bottomLeft = vertices[bli];
				Vert3D bottomRight = vertices[bri];

				//top left face
				SVec3 ab = topLeft.pos - topRight.pos;
				SVec3 ac = bottomLeft.pos - topRight.pos;
				SVec3 normal = ab.Cross(ac);

				faces[row].push_back(std::make_pair(SVec3(tli, tri, bli), normal));

				//bottom right face
				ab = topRight.pos - bottomRight.pos;
				ac = bottomLeft.pos - bottomRight.pos;
				normal = ab.Cross(ac);

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

				if (fabs(currentNormal.Length()) > 0.0001f)
					currentNormal.Normalize();

				vertices[index].normal = currentNormal;
			}
		}


		//assign indices for directX
		indices.clear();

		indices.reserve(faces.size() * faces.size() * 2);

		for (auto row : faces) {
			for (auto face : row) {
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



	void Terrain::Draw(ID3D11DeviceContext* dc, Shader& s, const SMatrix& mt, const SMatrix& vt, const SMatrix& pt, const PointLight& dLight, float deltaTime, SVec3 eyePos)
	{
		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		MatrixBufferType* dataPtr;
		LightBufferType* dataPtr2;
		VariableBufferType* dataPtr3;

		// Lock the constant matrix buffer so it can be written to.
		result = dc->Map(s.m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;

		dataPtr = (MatrixBufferType*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.

		SMatrix mT = mt.Transpose();
		SMatrix vT = vt.Transpose();
		SMatrix pT = pt.Transpose();

		// Copy the matrices into the constant buffer.
		dataPtr->world = mT;
		dataPtr->view = vT;
		dataPtr->projection = pT;

		// Unlock the constant buffer.
		dc->Unmap(s.m_matrixBuffer, 0);

		bufferNumber = 0;	// Set the position of the constant buffer in the vertex shader.
		dc->VSSetConstantBuffers(bufferNumber, 1, &s.m_matrixBuffer);	// Now set the constant buffer in the vertex shader with the updated values.
		//END MATRIX BUFFER



		result = dc->Map(s.m_variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;

		dataPtr3 = (VariableBufferType*)mappedResource.pData;

		dataPtr3->deltaTime = deltaTime;
		dataPtr3->padding = SVec3();

		dc->Unmap(s.m_variableBuffer, 0);

		bufferNumber = 1;
		dc->VSSetConstantBuffers(bufferNumber, 1, &s.m_variableBuffer);



		result = dc->Map(s.m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;


		dataPtr2 = (LightBufferType*)mappedResource.pData;
		dataPtr2->alc = dLight.alc;
		dataPtr2->ali = dLight.ali;
		dataPtr2->dlc = dLight.dlc;
		dataPtr2->dli = dLight.dli;
		dataPtr2->slc = dLight.slc;
		dataPtr2->sli = dLight.sli;
		dataPtr2->pos = dLight.pos;
		dataPtr2->ePos = SVec4(eyePos.x, eyePos.y, eyePos.z, 1.0f);

		dc->Unmap(s.m_lightBuffer, 0);

		bufferNumber = 0;

		unsigned int stride = sizeof(Vert3D);
		unsigned int offset = 0;

		dc->VSSetShader(s.m_vertexShader, NULL, 0);
		dc->PSSetShader(s.m_pixelShader, NULL, 0);

		dc->PSSetConstantBuffers(bufferNumber, 1, &s.m_lightBuffer);
		dc->PSSetSamplers(0, 1, &s.m_sampleState);

		dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dc->PSSetSamplers(0, 1, &s.m_sampleState);
		dc->IASetInputLayout(s.m_layout);

		dc->DrawIndexed(indices.size(), 0, 0);

		

		//dc->PSSetShaderResources(0, 1, &(unbinder[0]));
	}



	void Terrain::fault(const SRay& line, float displacement) 
	{
		float adjX = line.position.x * xScale;
		float adjZ = line.position.z * zScale;

		for (int i = 0; i < vertices.size(); ++i)
			if (line.direction.z * (vertices[i].pos.x - adjX) - line.direction.x * (vertices[i].pos.z - adjZ) > 0)
				vertices[i].pos.y += displacement;
	}


	//keep decay under 1 for reasonable results? Still, something fun could happen otherwise...
	void Terrain::TerraSlash(const SRay& line, float displacement, unsigned int steps, float decay)
	{
		Chaos c;

		fault(line, displacement);

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

			fault(SRay(randomAxis, randomDir), displacement);
		}
	}



	void Terrain::CircleOfScorn(const SRay& line, float displacement, unsigned int steps, float decay) 
	{

	
	}

}