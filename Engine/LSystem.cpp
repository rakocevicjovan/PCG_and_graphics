#include "LSystem.h" 
#include "Geometry.h"


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



	Model LSystem::genModel(ID3D11Device* device, float length, float radius, const float lengthConstriction, const float radiusConstriction, float pitch, float yaw)
	{
		//initial mesh config
		const float c_radius = radius;
		const float c_length = length;
		float accRadConstriction = 1.f;
		float accLenConstriction = 1.f;

		//for traversing the tree
		SMatrix orientation(SMatrix::Identity);
		SVec3 pos(0.f, 0.f, 0.f);
		SVec3 dir(0.f, 1.f, 0.f);
		SVec3 nextPos(0.f, 0.f, 0.f);

		//branching
		std::vector<SVec3> storedPos;
		std::vector<SMatrix> storedOri;
		std::vector<unsigned int> storedIndices;
		std::vector<float> storedLengths;
		std::vector<float> storedRadii;

		Geometry sphere, tube;
		sphere.GenSphere(1.f);
		Geometry tempSphere = sphere;	//avoid recalculating everything for the sphere, just copy it, scale and translate

		for (int i = 0; i < _current.size(); ++i)
		{
			char c = _current[i];
			char n = _current[i + 1];

			SVec3 curDir;

			nextPos = pos;
			SVec3 rotated = SVec3::Transform(dir, orientation);

			length = c_length * accLenConstriction;
			radius = c_radius * accRadConstriction;

			switch (c)
			{
			case 'F':

				nextPos += length * rotated;

				tube.Clear();
				//add tube
				if(n == ']')
					tube.GenTube(radius, length, 24, 10, 0.f);
				else if(n == '[')
					tube.GenTube(radius, length, 24, 10, radiusConstriction);
				else
					tube.GenTube(radius, length, 24, 10, 1.f);

				for (int i = 0; i < tube.positions.size(); ++ i)	//auto& vp : g.positions
				{
					Math::RotateVecByMat(tube.positions[i], orientation);
					Math::RotateVecByMat(tube.normals[i], orientation);
					tube.positions[i] += pos; //+ curDir * 0.5f;
				}

				tree.meshes.push_back(Mesh(tube, device));

				for (int i = 0; i < sphere.positions.size(); ++i)
				{
					tempSphere.positions[i] = sphere.positions[i] * radius * 1.05f;
					tempSphere.positions[i] += pos;
				}

				tree.meshes.push_back(Mesh(tempSphere, device));
				//accumulatedDecay *= decay;

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
				orientation *= SMatrix::CreateFromAxisAngle(orientation.Up(), -yaw);
				break;

			case '[':
				storedPos.push_back(pos);
				storedOri.push_back(orientation);
				storedRadii.push_back(accRadConstriction);
				storedLengths.push_back(accLenConstriction);
				accRadConstriction *= radiusConstriction;
				accLenConstriction *= lengthConstriction;
				break;

			case ']':
				nextPos = storedPos.back();
				storedPos.pop_back();

				orientation = storedOri.back();
				storedOri.pop_back();
				
				accRadConstriction = storedRadii.back();
				storedRadii.pop_back();

				accLenConstriction = storedLengths.back();
				storedLengths.pop_back();
				
				break;

			default:
				std::cout << c << std::endl;
				break;
			}
			pos = nextPos;
		}


		return tree;
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



	void LSystem::drawAsLines(ID3D11DeviceContext* dc, ShaderLight& s, 
			const SMatrix& mt, const SMatrix& vt, const SMatrix& pt, 
			const PointLight& dLight, float deltaTime, SVec3 eyePos)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MatrixBuffer* dataPtr;
		LightBuffer* dataPtr2;
		VariableBuffer* dataPtr3;

		SMatrix mT = mt.Transpose();
		SMatrix vT = vt.Transpose();
		SMatrix pT = pt.Transpose();

		// Lock the constant matrix buffer so it can be written to.
		if (FAILED(dc->Map(s._matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))return;
		dataPtr = (MatrixBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
		dataPtr->world = mT;
		dataPtr->view = vT;
		dataPtr->projection = pT;
		dc->Unmap(s._matrixBuffer, 0);
		dc->VSSetConstantBuffers(0, 1, &s._matrixBuffer);	


		if (FAILED(dc->Map(s._variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) return;
		dataPtr3 = (VariableBuffer*)mappedResource.pData;
		dataPtr3->deltaTime = deltaTime;
		dataPtr3->padding = SVec3();
		dc->Unmap(s._variableBuffer, 0);
		dc->VSSetConstantBuffers(1, 1, &s._variableBuffer);

		if (FAILED(dc->Map(s._lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) return;
		dataPtr2 = (LightBuffer*)mappedResource.pData;
		dataPtr2->alc = dLight.alc;
		dataPtr2->ali = dLight.ali;
		dataPtr2->dlc = dLight.dlc;
		dataPtr2->dli = dLight.dli;
		dataPtr2->slc = dLight.slc;
		dataPtr2->sli = dLight.sli;
		dataPtr2->pos = dLight.pos;
		dataPtr2->ePos = SVec4(eyePos.x, eyePos.y, eyePos.z, 1.0f);
		dc->Unmap(s._lightBuffer, 0);
		dc->PSSetConstantBuffers(0, 1, &s._lightBuffer);

		unsigned int stride = sizeof(Vert3D);
		unsigned int offset = 0;

		dc->VSSetShader(s._vertexShader, NULL, 0);
		dc->PSSetShader(s._pixelShader, NULL, 0);

		dc->PSSetSamplers(0, 1, &s._sampleState);

		dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		dc->PSSetSamplers(0, 1, &s._sampleState);
		dc->IASetInputLayout(s._layout);

		dc->DrawIndexed(indices.size(), 0, 0);
	}

}