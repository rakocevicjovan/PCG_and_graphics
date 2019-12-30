#pragma once

#include <string>
#include <map>
#include <vector>
#include <d3d11.h>

#include "assimp\Importer.hpp"	
#include "assimp\scene.h"
#include "assimp\postprocess.h" 
#include "Mesh.h"

class Collider;

namespace Procedural { class Terrain; }

class Model : public Resource
{
private:

	bool processNode(ID3D11Device* device, aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform, float rUVx, float rUVy);
	
	Mesh processMesh(ID3D11Device* device, aiMesh *mesh, const aiScene *scene, unsigned int ind, aiMatrix4x4 parentTransform, float rUVx, float rUVy);
	
	bool loadMaterialTextures(std::vector<Texture>& textures, const aiScene* scene, aiMaterial *mat, aiTextureType type, std::string typeName);
	
	bool loadEmbeddedTexture(Texture& texture, const aiScene* scene, UINT index);
	
	SVec3 calculateTangent(const std::vector<Vert3D>& vertices, const aiFace& face);


public:
	std::vector<Mesh> meshes;

	SMatrix transform;

	Collider* collider;		//remove this eventually when game object becomes better defined... used model for it so far...

	Model() {}
	Model(const std::string& path);
	Model(const Collider& collider, ID3D11Device* device);
	Model(const Procedural::Terrain& terrain, ID3D11Device* device);
	~Model();

	bool LoadModel(ID3D11Device* device, const std::string& path, float rUVx = 1, float rUVy = 1);

	template <class FlexibleShaderType>
	void Draw(ID3D11DeviceContext* dc, FlexibleShaderType& shader)
	{
		for (unsigned int i = 0; i < this->meshes.size(); i++)
			this->meshes[i].draw(dc, shader);
	}

	void DrawInstanced(ID3D11DeviceContext* dc, InstancedShader& shader)
	{
		for (unsigned int i = 0; i < this->meshes.size(); i++)
			this->meshes[i].draw(dc, shader);
	}

	/* 
	//it seems that this is not compatible with the placement new... or at least not the way I tried it
	//because placement new expects this same signature... so it's either or
	void* operator new(size_t size)
	{
		std::cout << "MODEL NEW OVERRIDE!" << std::endl;
		void* p = ::new Model();
		return p;
	}*/
};


/*
class ModelInstance
{
public:
	Model* model;
	SMatrix transform;
};
*/