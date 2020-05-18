#pragma once
#include "Model.h"
#include "FileUtilities.h"
#include "GuiBlocks.h"
#include "Texture.h"

class AssimpPreview
{
private:

	Assimp::Importer _importer;

	std::string _path;
	const aiScene* _scene;

	UINT _meshIndex;

public:

	Model _model;



	bool loadAiScene(ID3D11Device* device, const std::string& path, UINT inFlags)
	{
		_path = path;
		_meshIndex = 0;

		assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

		unsigned int pFlags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_PreTransformVertices |
			aiProcess_ConvertToLeftHanded;

		_scene = _importer.ReadFile(path, pFlags);

		if (!_scene || _scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode)
		{
			std::string errString("Assimp error:" + std::string(_importer.GetErrorString()));
			OutputDebugStringA(errString.c_str());
			return false;
		}

		_model.LoadModel(device, path);

		return true;
	}
	


	void displayAiScene()
	{
		_meshIndex = 0;

		ImGui::Begin("Scene");

		if (ImGui::TreeNode("Node tree"))
		{
			printaiNode(_scene->mRootNode, _scene, _scene->mRootNode->mTransformation);

			ImGui::TreePop();
		}

		ImGui::End();
	}



	bool printaiNode(aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform)
	{
		std::string nodeName("Node: ");
		nodeName += node->mName.C_Str();

		if (ImGui::TreeNode(nodeName.c_str()))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.6, 0., 1., 1.));

			aiMatrix4x4 concatenatedTransform = parentTransform * node->mTransformation;

			if (ImGui::TreeNode("Concatenated transform: "))
			{
				displayTransform(SMatrix(&concatenatedTransform.a1));
				ImGui::TreePop();
			}
			

			ImGui::Text("Mesh count: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(node->mNumMeshes).c_str());

			if (node->mNumMeshes > 0)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0., 1., 0., 1.));
				if (ImGui::TreeNode("Meshes"))
				{
					for (unsigned int i = 0; i < node->mNumMeshes; i++)
					{
						aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
						++_meshIndex;

						std::string meshName("Mesh name and index: ");
						meshName += mesh->mName.C_Str();
						meshName += " ";
						meshName += std::to_string(_meshIndex);

						if (ImGui::TreeNode(meshName.c_str()))
						{
							ImGui::PushID(i);
							printAiMesh(mesh, concatenatedTransform);
							ImGui::PopID();
							ImGui::TreePop();
						}

					}
					ImGui::TreePop();
				}
				ImGui::PopStyleColor();
			}


			ImGui::Separator();

			ImGui::Text("Children count: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(node->mNumChildren).c_str());

			if (node->mNumChildren > 0)
			{
				if (ImGui::TreeNode("Children"))
				{
					for (unsigned int i = 0; i < node->mNumChildren; i++)
					{
						ImGui::PushID(i);
						printaiNode(node->mChildren[i], scene, concatenatedTransform);
						ImGui::PopID();
					}
					ImGui::TreePop();
				}	
			}

			ImGui::PopStyleColor();
			ImGui::TreePop();
		}
		

		return true;
	}



	void printAiMesh(aiMesh* mesh, aiMatrix4x4 parentGlobal)
	{
		UINT numUVChannels = mesh->GetNumUVChannels();
		UINT* numUVComponents = mesh->mNumUVComponents;

		bool hasTexCoords = mesh->mTextureCoords[0];

		ImGui::BeginGroup();

		ImGui::Text("UVs");

		ImGui::Text("Nr. of UV channels: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(numUVChannels).c_str());

		ImGui::Text("Nr. of UV components per channel: ");

		ImGui::Indent();
		for (int i = 0; i < numUVChannels; i++)
		{
			ImGui::Text(std::to_string(numUVComponents[i]).c_str());
		}
		ImGui::Unindent();

		ImGui::EndGroup();

		ImGui::Separator();

		//  Get a minimum bounding sphere, useful for model loading but not needed here
		/*

		//float maxDist = 0.f;
		//Vert3D vertex;

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			
			vertex.pos = SVec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

			float curDist = vertex.pos.LengthSquared();
			if (maxDist < curDist)
				maxDist = curDist;

			vertex.normal = SVec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			//vertex.normal.Normalize();	//not sure if required, should be so already

			vertex.texCoords = hasTexCoords ? SVec2(aiMesh->mTextureCoords[0][i].x * rUVx, aiMesh->mTextureCoords[0][i].y * rUVy) : SVec2::Zero;

			mesh._vertices.push_back(vertex);
			
		}

		//maxDist = sqrt(maxDist);
		*/

		UINT indexCount = 0u;

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			//populate indices from faces
			for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
				++indexCount;	//_indices.push_back(face.mIndices[j]);
		}

		ImGui::BeginGroup();

		ImGui::Text("Vertex count: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(mesh->mNumVertices).c_str());

		ImGui::Text("Index count: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(indexCount).c_str());

		ImGui::Text("Face count: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(mesh->mNumFaces).c_str());
	
		ImGui::Text("Has tangents and bitangents: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(mesh->HasTangentsAndBitangents()).c_str());

		ImGui::EndGroup();

		ImGui::Separator();

		printAiMaterial(mesh);
	}



	void printAiMaterial(aiMesh* mesh)
	{
		ImGui::BeginGroup();

		ImGui::Text("Material: ");
		ImGui::Indent();

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = _scene->mMaterials[mesh->mMaterialIndex];

			// Diffuse maps
			printMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", DIFFUSE);

			//  Normal maps
			printMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", NORMAL);

			// Specular maps
			printMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", SPECULAR);

			// Shininess maps
			printMaterialTextures(material, aiTextureType_SHININESS, "texture_shininess", SHININESS);

			// Opacity maps
			printMaterialTextures(material, aiTextureType_OPACITY, "texture_opacity", OPACITY);

			// Displacement maps
			printMaterialTextures(material, aiTextureType_DISPLACEMENT, "texture_disp", DISPLACEMENT);

			// Ambient occlusion maps
			printMaterialTextures(material, aiTextureType_AMBIENT, "texture_AO", AMBIENT);

			// Other maps
			printMaterialTextures(material, aiTextureType_UNKNOWN, "texture_other", OTHER);

			// Weird properties... that I never really saw trigger
			printMaterialTextures(material, aiTextureType_NONE, "texture_property", OTHER);
		}

		ImGui::Unindent();
		ImGui::EndGroup();
	}



	void printMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, TextureRole role)
	{
		UINT numThisType = mat->GetTextureCount(type);
		typeName += " count: ";
		typeName += std::to_string(numThisType);

		ImGui::Text(typeName.c_str());

		ImGui::Indent();

		//iterate all textures of relevant related to the material
		for (unsigned int i = 0; i < numThisType; ++i)
		{
			// Try to load this texture from file
			aiString obtainedTexturePath;
			mat->GetTexture(type, i, &obtainedTexturePath);

			// This assumes files are exported with relative paths... which is a big if, considering artists can't seem to grasp the concept
			
			std::string modelFolderPath = _path.substr(0, _path.find_last_of("/\\")) + "\\";
			std::string texPath = modelFolderPath + std::string(obtainedTexturePath.data);
			std::string texName = std::filesystem::path(std::string(obtainedTexturePath.C_Str())).filename().string();

			bool loaded = FileUtils::fileExists(texPath);
			
			if (loaded)
			{
				ImGui::Text("Path: ");
				ImGui::SameLine();
				ImGui::Text(texPath.c_str());
			}
			else
			{
				aiTexture* aiTex;

				int embeddedIndex = atoi(obtainedTexturePath.C_Str() + sizeof(char));	//skip the * with + sizeof(char), also switch to std::to_integer()

				if (_scene->mTextures)
				{
					aiTex = _scene->mTextures[embeddedIndex];
					loaded = (aiTex != nullptr);
				}

				if (loaded)
				{
					ImGui::Text("This texture is embedded at index ");
					ImGui::SameLine();
					ImGui::Text(std::to_string(embeddedIndex).c_str());
				}
				else
				{
					ImGui::Text("Path: ");
					ImGui::SameLine();
					ImGui::Text(texPath.c_str());

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1., 0., 0., 1.));
					ImGui::Text(" ( WARNING: NOT FOUND! )");
					ImGui::PopStyleColor();

					// Try to apply the fix
					std::filesystem::directory_entry dumbassArtist;
					if (FileUtils::findFile(modelFolderPath, texName, dumbassArtist))
					{
						ImGui::Indent();
						ImGui::Text("Proposed path: ");
						ImGui::SameLine();
						ImGui::Text(dumbassArtist.path().string().c_str());
						ImGui::Unindent();
					}
				}

			}
		}

		ImGui::Unindent();
	}



	void printAnimations(aiScene* scene)
	{
		ImGui::Begin("Animation data");



		ImGui::End();
	}

};