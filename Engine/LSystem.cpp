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



	void LSystem::removeRule(char left, std::string right)
	{
		RewriteRule inRule(left, right);
		_rules.erase(std::remove(_rules.begin(), _rules.end(), inRule), _rules.end());
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

		float sqrtRadCon = sqrt(radiusConstriction);
		float sqrtLenCon = sqrt(lengthConstriction);


		//for traversing the tree
		SMatrix orientation(SMatrix::Identity);
		SVec3 pos(0.f, 0.f, 0.f);
		SVec3 dir(0.f, 1.f, 0.f);
		SVec3 nextPos(0.f, 0.f, 0.f);

		//branching
		std::vector<SVec3> storedPos;
		std::vector<SMatrix> storedOri;
		std::vector<float> storedLengths;
		std::vector<float> storedRadii;

		Geometry sphere, tube;
		sphere.GenSphere(1.f);
		Geometry tempSphere = sphere;	//avoid recalculating everything for the sphere, just copy it, scale and translate

		for (int i = 0; i < _current.size(); ++i)
		{
			nextPos = pos;

			char c = _current[i];
			char n = _current[i + 1];
			bool isEnd = n == ']';
			
			SVec3 rotated = SVec3::Transform(dir, orientation);
			float branchTipRadiusPercent = 1.f;

			if (isEnd)	//branch is at the end of the branch - make it a briar
				branchTipRadiusPercent = .1f;

			switch (c)
			{
			case 'F':

				nextPos = pos + length * rotated;

				tube.Clear();
				tube.GenTube(radius, length, 8, 4, branchTipRadiusPercent);

				//add tube
				for (int i = 0; i < tube.positions.size(); ++i)
				{
					Math::RotateVecByMat(tube.positions[i], orientation);
					Math::RotateVecByMat(tube.normals[i], orientation);
					tube.positions[i] += pos;
				}

				//add sphere
				tree.meshes.emplace_back(tube, device, false, true);

				if (!isEnd)
				{
					for (int i = 0; i < sphere.positions.size(); ++i)
					{
						tempSphere.positions[i] = sphere.positions[i] * branchTipRadiusPercent * radius;
						tempSphere.positions[i] += nextPos;
					}
					tree.meshes.emplace_back(tempSphere, device, false, true);
				}

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
				storedPos.emplace_back(pos);
				storedOri.emplace_back(orientation);
				storedRadii.emplace_back(accRadConstriction);
				storedLengths.emplace_back(accLenConstriction);
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

			length = c_length * accLenConstriction;
			radius = c_radius * accRadConstriction;

			pos = nextPos;
		}

		tree.textures_loaded.emplace_back(device, "../Textures/Bark/diffuse.jpg");
		tree.textures_loaded.emplace_back(device, "../Textures/Bark/normal.jpg");

		Mesh finalMesh;
		finalMesh.textures = tree.textures_loaded;
		int totalVerts = 0, totalInds = 0;

		for (auto&m : tree.meshes)
		{
			totalVerts += m.vertices.size();
			totalInds += m.indices.size();
		}
			
		finalMesh.vertices.reserve(totalVerts);
		finalMesh.indices.reserve(totalInds);

		for (auto&m : tree.meshes)
		{
			int accIndSize = finalMesh.vertices.size();
			for (auto& ind : m.indices)
				ind += accIndSize;
			finalMesh.indices.insert(finalMesh.indices.end(), m.indices.begin(), m.indices.end());
			finalMesh.vertices.insert(finalMesh.vertices.end(), m.vertices.begin(), m.vertices.end());
		}
		//finalMesh.setupMesh(device);
		tree.meshes.clear();
		tree.meshes.emplace_back(finalMesh.vertices, finalMesh.indices, tree.textures_loaded, device, 0);
		
		return tree;
	}



	Model LSystem::genFlower(ID3D11Device* device, Model * petalModel, float stalkSegmentLength, float stalkRadius, float deescalator, float angle, float tilt)
	{
		SMatrix orientation(SMatrix::Identity);
		SVec3 pos(0.f, 0.f, 0.f);
		SVec3 dir(0.f, 1.f, 0.f);
		SVec3 nextPos(0.f, 0.f, 0.f);

		//branching
		std::vector<SVec3> storedPos;
		std::vector<SMatrix> storedOri;
		std::vector<unsigned int> storedIndices;


		//matrix that rotates the petal around the stalk
		SMatrix petalRotMat = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), angle);

		Geometry tube;
		tube.GenTube(1.f, stalkSegmentLength, 5, 2, 1.f);
		
		Mesh stalkMesh(tube, device);
		Mesh tempStalkMesh;

		Mesh petalMesh = petalModel->meshes[0];
		Mesh tempPetalMesh = petalMesh;
		
		Model result;

		for (int i = 0; i < _current.size(); ++i)
		{
			nextPos = pos;

			char c = _current[i];
			
			SVec3 rotated = SVec3::Transform(dir, orientation);

			switch (c)
			{
			case 'D':	//advance a floor up

				nextPos = pos + stalkSegmentLength * rotated;
				tilt *= deescalator;

			case 'S':	//create stalk

				tempStalkMesh = stalkMesh;
				for (int i = 0; i < stalkMesh.vertices.size(); ++i)
				{
					tempStalkMesh.vertices[i].pos = SVec3::Transform(stalkMesh.vertices[i].pos, orientation);
					tempStalkMesh.vertices[i].pos += nextPos;
				}

				result.meshes.push_back(tempStalkMesh);
				break;

			case 'P':

				for (int i = 0; i < petalMesh.vertices.size(); ++i)
				{
					tempPetalMesh.vertices[i].pos = SVec3::Transform(petalMesh.vertices[i].pos, orientation);
					tempPetalMesh.vertices[i].pos += nextPos;
				}
					
				result.meshes.push_back(tempPetalMesh);
				
				break;

			case '+':
				orientation *= SMatrix::CreateFromAxisAngle(orientation.Forward(), -tilt);

			case '*':
				orientation *= petalRotMat;
				break;

			case '[':
				storedPos.emplace_back(pos);
				storedOri.emplace_back(orientation);
				break;

			case ']':
				nextPos = storedPos.back();
				storedPos.pop_back();
				orientation = storedOri.back();
				storedOri.pop_back();
				break;

			default:
				std::cout << c << std::endl;
				break;
			}

			pos = nextPos;
		}

		for (Mesh& m : result.meshes)
			m.setupMesh(device);

		return result;
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


/*
bool isParentOfBranch = false;
if (!isEnd)
{
	size_t nextBranch = _current.find('F', i + 1);
	if (nextBranch != std::string::npos)
	{
		std::string untilNextBranch = _current.substr(i + 1, nextBranch - i - 1);
		isParentOfBranch = untilNextBranch.find('[', 0) != std::string::npos;
	}
}
*/
//if (isParentOfBranch)	//branches sprout from the end of this branch... not sure what to do with that
	//branchTipRadiusPercent = 1.f;//sqrtRadCon;