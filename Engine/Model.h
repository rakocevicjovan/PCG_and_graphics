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

class Model{

public:

	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;

	std::string directory;
	std::string name;

	Joint rootJoint;

	SMatrix transform;

	Model() {}

	Model(const std::string& path){
		this->name = path;
	}


	bool LoadModel(ID3D11Device* device, const std::string& path){

		//@TODO could be fucky...
		assert(fileExists(path) && "File does not exist! ...probably.");

		// Read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

		// Check for errors
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
			std::string errString("Assimp error:" + std::string(importer.GetErrorString()) );
			OutputDebugStringA(errString.c_str());
			return false;
		}

		directory = path.substr(0, path.find_last_of('/'));	
		name = path.substr(path.find_last_of('/')+1, path.size());

		processNode(device, scene->mRootNode, scene);
		return true;
	}


		
	// Processes a node in a recursive fashion. Processes each individual mesh located at the node  
	//and repeats this process on its children nodes (if any).
	bool processNode(ID3D11Device* device, aiNode* node, const aiScene* scene) {

		//BONE CODE, MIGHT SEPARATE COMPLETELY FROM STATIC MESHES

		// Process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++){
			// The node object only contains indices to index the actual objects in the scene.
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			meshes.push_back(processMesh(device, mesh, scene));
		}

		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++){
			this->processNode(device, node->mChildren[i], scene);
		}
		return true;
	}




	//reads in vertices, indices and texture UVs of a mesh
	Mesh processMesh(ID3D11Device* device, aiMesh *mesh, const aiScene *scene){
		// Data to fill
		std::vector<Vert3D> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		bool hasTexCoords = false;

		///THIS COULD BE AN ERROR! WATCH OUT!
		if (mesh->mTextureCoords[0])
			hasTexCoords = true;

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++){
			Vert3D vertex;
			
			vertex.pos = SVec3(mesh->mVertices[i].x, mesh->mVertices[i].y , mesh->mVertices[i].z);
			vertex.normal = SVec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

			// Texture Coordinates
			if (hasTexCoords) { // Does the mesh contain texture coordinates?
				// A vertex can contain up to 8 different texture coordinates. We make the assumption that we won't
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vertex.texCoords = SVec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
				auto a = mesh->mTextureCoords[0];

			} else 
				vertex.texCoords = SVec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++){
			aiFace face = mesh->mFaces[i];
			// Retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++){
				indices.push_back(face.mIndices[j]);
			}
		}

		// Process materials
		if (mesh->mMaterialIndex >= 0){
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// 1. Diffuse maps
			std::vector<Texture> diffuseMaps = this->loadMaterialTextures(device, scene, material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. Specular maps
			std::vector<Texture> specularMaps = this->loadMaterialTextures(device, scene, material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		}			

		return Mesh(vertices, indices, textures, device);
	}



	BonedMesh processBonedMesh(ID3D11Device* device, aiMesh *mesh, const aiScene *scene) {

		std::vector<BonedVert> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		bool hasTexCoords = false;

		///THIS COULD BE AN ERROR! WATCH OUT!
		if (mesh->mTextureCoords[0])
			hasTexCoords = true;

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			BonedVert vertex;

			// Positions
			vertex.vert.pos = SVec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

			// Normals
			vertex.vert.normal = SVec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

			// Texture Coordinates
			if (hasTexCoords) {
				vertex.vert.texCoords = SVec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
				auto a = mesh->mTextureCoords[0];
			}
			else 
				vertex.vert.texCoords = SVec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			// Retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// Process materials
		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// Diffuse: texture_diffuseN
			// Specular: texture_specularN
			// Normal: texture_normalN

			// 1. Diffuse maps XXXXXXXXXXXXXXXXXXXX
			std::vector<Texture> diffuseMaps = this->loadMaterialTextures(device, scene, material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			// 2. Specular maps
			std::vector<Texture> specularMaps = this->loadMaterialTextures(device, scene, material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		}

		auto joints = loadBones(*mesh, vertices);

		return BonedMesh(vertices, indices, textures, joints);
	}



	std::vector<Texture> loadMaterialTextures(ID3D11Device* device, const aiScene* scene, aiMaterial *mat, aiTextureType type, std::string typeName){
			
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++){

			aiString str;
			boolean skip = false;

			mat->GetTexture(type, i, &str);
				
			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture			
			for (unsigned int j = 0; j < textures_loaded.size(); j++){
				if (aiString(textures_loaded[j].fileName) == str){
					textures.push_back(textures_loaded[j]);
					skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}

			if (!skip){   // If texture hasn't been loaded already, load it

				std::string fPath = directory + "/" + std::string(str.data);
				Texture texture(device, fPath);
				texture.typeName = typeName;

				//texture.Bind(type);
				bool loaded = texture.Load();
					
				if (!loaded) {
					loaded = this->LoadGLTextures(device, textures, scene, fPath, type, typeName);	//for embedded textures

					if (!loaded)
						std::cout << "Texture did not load!" << std::endl;

					return textures;
				}

				textures.push_back(texture);
				textures_loaded.push_back(texture); 
				// Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}

		return textures;
	}

	bool LoadGLTextures(ID3D11Device* device, std::vector<Texture>& textures, const aiScene* scene, std::string& fPath, aiTextureType type, std::string& typeName) {

		if (scene->HasTextures()) {

			for (size_t ti = 0; ti < scene->mNumTextures; ti++) {

				Texture texture(device, fPath);
				texture.typeName = typeName;

				texture.LoadFromMemory(scene->mTextures[ti], device);

				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}

			return true;
		}
		return false;	//has no textures (at least if you ask assimp), so loading none is fine
	}


	/*... 1:00 AM revelation!
	Bone indices in my data are completely dictated by me 
	and have fuck all to do with bone indices in assimp, 
	the only thing connecting bones to vertices are NAMES OF BONES -> INDICES OF VERTICES.*/

	//loads joint influence per vertex data, as well as the tree itself
	std::vector<Joint> Model::loadBones(const aiMesh& aiMesh, std::vector<BonedVert>& verts) {

		if (!aiMesh.HasBones())
			return {};

		std::vector<Joint> result;

		for (unsigned int i = 0; i < aiMesh.mNumBones; i++) {

			auto bone = aiMesh.mBones[i];		//bone at index i in assimp data, NOT THE INDEX WE ARE USING!!!
			int boneIndex = result.size();		//index to be stored in my mesh data

			//adds the bone, gives it an index by which it can be found... 
			//MAYBE ASSIMP ID SHOULD BE THE INDEX??? @TODO FIGURE IT OUT
			Joint j(boneIndex, std::string(bone->mName.data), SMatrix( bone->mOffsetMatrix[0] ));
			result.push_back(j);

			/*IMPORTANT! right now, the joints here have no parents or children therefore in order to correctly
			calculate anything at runtime, I need to establish these relationships by going through the joint vector of this mesh, 
			finding all the joint names and finding their parents and children in the aiNode hierarchy.
			*/

			//adds vertex bone data, saing which (up to MBVP, which is 4 right now) bones affect the vertex
			//vertices are indexed per mesh, not per model (I hope?)
			//bones are indexed by the number boneIndex, which is MY index (not assimp's) in MY vector of bones
			for (unsigned int j = 0; j < bone->mNumWeights; j++) {
				int curVertex = bone->mWeights[j].mVertexId;
				//verts.at(curVertex).joints.addBoneData(boneIndex, bone->mWeights[j].mWeight);	@TODO WTF
			}

		}

		return result;

	}

	std::vector<std::string> addBoneNames() {

	}



	// Draws the model, and thus all its meshes
	void Draw(ID3D11DeviceContext* dc, Shader& shader) {
		for (unsigned int i = 0; i < this->meshes.size(); i++)
			this->meshes[i].draw(dc, shader);
	}


private:

	inline bool fileExists(const std::string& name) {
		std::ifstream f(name.c_str());
		return f.good();
	}
};