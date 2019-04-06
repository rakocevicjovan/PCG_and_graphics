#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <d3d11.h>

#include "assimp\Importer.hpp"	
#include "assimp\scene.h"
#include "assimp\postprocess.h" 
#include "Mesh.h"
#include "CollisionEngine.h"

namespace Procedural { class Terrain; }

class Model
{
public:
	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;

	std::string directory;
	std::string name;

	SMatrix transform;

	Collider collider;

	Model() {}
	Model(const std::string& path);
	Model(const Collider& collider, ID3D11Device* device);
	Model(const Procedural::Terrain& terrain, ID3D11Device* device);
	~Model();


	bool LoadModel(ID3D11Device* device, const std::string& path, float rUVx = 1, float rUVy = 1);
	bool processNode(ID3D11Device* device, aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform, float rUVx, float rUVy);
	Mesh processMesh(ID3D11Device* device, aiMesh *mesh, const aiScene *scene, unsigned int ind, aiMatrix4x4 parentTransform, float rUVx, float rUVy);
	std::vector<Texture> loadMaterialTextures(ID3D11Device* device, const aiScene* scene, aiMaterial *mat, aiTextureType type, std::string typeName);
	bool LoadEmbeddedTextures(ID3D11Device* device, std::vector<Texture>& textures, const aiScene* scene, std::string& fPath, aiTextureType type, std::string& typeName);
	SVec3 calculateTangent(const std::vector<Vert3D>& vertices, const aiFace& face);


	template <class FlexibleShaderType>
	void Draw(ID3D11DeviceContext* dc, FlexibleShaderType& shader)
	{
		for (unsigned int i = 0; i < this->meshes.size(); i++)
			this->meshes[i].draw(dc, shader);
	}

private:
	inline bool fileExists(const std::string& name);
};