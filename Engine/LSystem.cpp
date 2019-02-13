#include "LSystem.h" 




namespace Procedural
{
	LSystem::LSystem(std::string axiom) : _axiom(axiom)
	{

	}



	LSystem::~LSystem()
	{
	}



	void LSystem::reseed(std::string axiom)
	{
		_axiom = axiom;
		_current = axiom;
	}



	void LSystem::rewrite(unsigned int steps)
	{

		if (_current == "")
			_current.append(_axiom);

		for (unsigned int i = 0; i < steps; i++)
		{
			for (char c : _current)
			{
				bool mapped = false;

				for (RewriteRule rwr : _rules)
				{
					if (c == rwr.l)
					{
						_next.append(rwr.r);
						mapped = true;
						break;
					}
				}

				if (!mapped)
					_next.push_back(c);
			}

			_current = _next;
			_next.clear();
		}
	}



	bool LSystem::addRule(char left, std::string right)
	{
		for (RewriteRule rwr : _rules)
			if (rwr.l == left)
				return false;

		_rules.push_back(RewriteRule(left, right));
		return true;
	}



	void LSystem::genVerts(float length, float decay, float pitch, float yaw)
	{
		SMatrix orientation(SMatrix::Identity);
		SVec3 pos(0.f, 0.f, 0.f);
		SVec3 nextPos(0.f, 0.f, 0.f);
		SVec3 dir(0.f, 1.f, 0.f);
		unsigned int index = 0u;

		std::vector<SVec3> storedPos;
		std::vector<SMatrix> storedOri;
		std::vector<unsigned int> storedIndices;

		for (char c : _current)
		{
			nextPos = pos;
			SVec3 rotated = SVec3::Transform(dir, orientation);
			switch (c)
			{
			case 'f':
				nextPos += length * rotated;
				verts.push_back(pos);
				verts.push_back(nextPos);

				indices.push_back(index);
				index = indices.size();
				indices.push_back(index);
				break;

			case '+':
				orientation *= SMatrix::CreateFromAxisAngle(orientation.Forward(), pitch);
				break;

			case '-':
				orientation *= SMatrix::CreateFromAxisAngle(orientation.Forward(), -pitch);
				break;

			case '*':
				orientation *= SMatrix::CreateFromAxisAngle(orientation.Up(), yaw);
				break;

			case '/':
				orientation *= SMatrix::CreateFromAxisAngle(orientation.Up(), - yaw);
				break;

			case '[':
				storedPos.push_back(pos);
				storedOri.push_back(orientation);
				storedIndices.push_back(index);
				break;

			case ']':
				nextPos = storedPos.back();
				storedPos.pop_back();

				orientation = storedOri.back();
				storedOri.pop_back();

				index = storedIndices.back();
				storedIndices.pop_back();
				break;

			default:
				break;
			}
			pos = nextPos;
		}
	}


	void LSystem::setUp(ID3D11Device* device)
	{
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT res;

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(Vert3D) * verts.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = verts.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		res = device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer);
		if (FAILED(res))
			exit(666);

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
			exit(666);
	}



	void LSystem::draw(ID3D11DeviceContext* dc, ShaderLight& s, 
			const SMatrix& mt, const SMatrix& vt, const SMatrix& pt, 
			const PointLight& dLight, float deltaTime, SVec3 eyePos)
	{
		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		MatrixBufferTypeA* dataPtr;
		LightBufferTypeA* dataPtr2;
		VariableBufferTypeA* dataPtr3;

		// Lock the constant matrix buffer so it can be written to.
		result = dc->Map(s.m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;

		dataPtr = (MatrixBufferTypeA*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.

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

		dataPtr3 = (VariableBufferTypeA*)mappedResource.pData;

		dataPtr3->deltaTime = deltaTime;
		dataPtr3->padding = SVec3();

		dc->Unmap(s.m_variableBuffer, 0);

		bufferNumber = 1;
		dc->VSSetConstantBuffers(bufferNumber, 1, &s.m_variableBuffer);



		result = dc->Map(s.m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;


		dataPtr2 = (LightBufferTypeA*)mappedResource.pData;
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
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		dc->PSSetSamplers(0, 1, &s.m_sampleState);
		dc->IASetInputLayout(s.m_layout);

		dc->DrawIndexed(indices.size(), 0, 0);



	}

}