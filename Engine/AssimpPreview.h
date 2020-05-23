#pragma once
#include "Model.h"
#include "FileUtilities.h"
#include "GuiBlocks.h"
#include "Texture.h"
#include "FileBrowser.h"



class AssimpPreview
{
private:

	Assimp::Importer _importer;

	std::string _path;

	const aiScene* _scene;

	UINT _meshIndex;	// Used to differentiate between mesh names because they tend to have same names

	Skeleton _skeleton;

public:



	bool loadAiScene(ID3D11Device* device, const std::string& path, UINT inFlags)
	{
		//static_assert(std::is_nothrow_move_constructible<AssimpPreview>::value, "MyType should be noexcept MoveConstructible");

		_path = path;
		_meshIndex = 0;

		unsigned int pFlags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_ConvertToLeftHanded;

		//AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES
		_importer.SetPropertyInteger(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, 0);

		_scene = AssimpWrapper::loadScene(_importer, path, pFlags);

		if (!_scene)
			return false;

		AssimpWrapper::loadBones(_scene, _scene->mRootNode, _skeleton);

		// This might be wrong
		const aiNode* skelRoot = AssimpWrapper::findSkeletonRoot(_scene->mRootNode, _skeleton);
		
		if (skelRoot)
			AssimpWrapper::linkSkeletonHierarchy(skelRoot, _skeleton);

		return true;
	}
	


	void displayAiScene(const std::string& sName)
	{
		_meshIndex = 0;

		if (ImGui::TreeNode("Node tree"))
		{
			printaiNode(_scene->mRootNode, _scene, _scene->mRootNode->mTransformation);

			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNode("Skeleton"))
		{
			printBone(_skeleton._root);
			ImGui::TreePop();
		}

		ImGui::Separator();

		printSceneAnimations();
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
				displayTransform(SMatrix(&concatenatedTransform.a1).Transpose());	// Careful
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
							ImGui::Separator();
							ImGui::TreePop();
						}

					}
					ImGui::TreePop();
				}
				ImGui::PopStyleColor();
			}


			ImGui::Separator();

			ImGui::Text("Child count: ");
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
						ImGui::Separator();
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
		ImGui::NewLine();

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
		ImGui::NewLine();

		printAiMaterial(mesh);
	}



	void printAiMaterial(aiMesh* mesh)
	{
		ImGui::Text("Material: ");
		ImGui::Indent();

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = _scene->mMaterials[mesh->mMaterialIndex];

			// Diffuse maps
			printMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", DIFFUSE);

			// Normal maps
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

					// Try to apply the fix for absolute paths to textures...
					std::filesystem::directory_entry artistPls;
					if (FileUtils::findFile(modelFolderPath, texName, artistPls))
					{
						ImGui::Indent();
						ImGui::Text("Proposed path: ");
						ImGui::SameLine();
						ImGui::Text(artistPls.path().string().c_str());
						ImGui::Unindent();
					}
				}

			}
		}

		ImGui::Unindent();
	}



	void printSceneAnimations()
	{
		if (!_scene->HasAnimations())
			return;

		if(ImGui::TreeNode("Animation list"))
		{
			for (int i = 0; i < _scene->mNumAnimations; ++i)
			{
				printAnimation(_scene->mAnimations[i]);
			}
			ImGui::TreePop();
		}
	}



	void printAnimation(aiAnimation* sceneAnimation)
	{
		int numChannels = sceneAnimation->mNumChannels;

		if (ImGui::TreeNode(sceneAnimation->mName.C_Str()))
		{
			ImGui::Text("Num channels: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(numChannels).c_str());

			for (int j = 0; j < numChannels; ++j)
			{
				aiNodeAnim* channel = sceneAnimation->mChannels[j];

				if (ImGui::TreeNode(channel->mNodeName.C_Str()))
				{
					printAnimationTrack(channel);
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}



	void printAnimationTrack(aiNodeAnim* channel)
	{
		ImGui::Text("Num scaling keys: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(channel->mNumScalingKeys).c_str());

		ImGui::Text("Num rotation keys: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(channel->mNumRotationKeys).c_str());

		ImGui::Text("Num position keys: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(channel->mNumPositionKeys).c_str());

		/*
		for (int c = 0; c < channel->mNumScalingKeys; c++)
		{
			double time = channel->mScalingKeys[c].mTime;
			aiVector3D chScale = channel->mScalingKeys[c].mValue;
			SVec3 scale = SVec3(chScale.x, chScale.y, chScale.z);
		}

		for (int b = 0; b < channel->mNumRotationKeys; b++)
		{
			double time = channel->mRotationKeys[b].mTime;
			aiQuaternion chRot = channel->mRotationKeys[b].mValue;
			SQuat rot = SQuat(chRot.x, chRot.y, chRot.z, chRot.w);
		}

		for (int a = 0; a < channel->mNumPositionKeys; a++)
		{
			double time = channel->mPositionKeys[a].mTime;
			aiVector3D chPos = channel->mPositionKeys[a].mValue;
			SVec3 pos = SVec3(chPos.x, chPos.y, chPos.z);
		}
		*/
	}



	void printBone(const Bone* bone)
	{
		if (ImGui::TreeNode(bone->name.c_str()))
		{
			ImGui::Text("Index: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(bone->index).c_str());

			for (Bone* cBone : bone->offspring)
				printBone(cBone);

			ImGui::TreePop();
		}
	}



	std::filesystem::path getPath() { return std::filesystem::path(_path); }
};