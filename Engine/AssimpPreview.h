#pragma once
#include "Model.h"
#include "FileUtilities.h"
#include "GuiBlocks.h"
#include "Texture.h"
#include "FileBrowser.h"
#include "Exporter.h"

#include "AnimationEditor.h"



class AssimpPreview
{
private:

	std::string _path;

	Assimp::Importer _importer;

	const aiScene* _scene;

	Exporter _exporter;


	Skeleton _skeleton;
	std::vector<Animation> _anims;
	std::vector<Texture> _embTextures;
	std::vector<Texture> _extTextures;



	// Put this...
	int _currentAnim;
	float _playbackSpeed;

	/*
	// ... here
	AnimationEditor _animEditor;	//@TODO sequencer and all that, not a priority yet
	Animation* _selectedAnim;
	*/

public:



	bool loadAiScene(ID3D11Device* device, const std::string& path, UINT inFlags)
	{
		_currentAnim = 0;
		_playbackSpeed = 1;

		_path = path;

		unsigned int pFlags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_ConvertToLeftHanded;

		_importer.SetPropertyBool(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);	// This doesn't work...

		_scene = AssimpWrapper::loadScene(_importer, path, pFlags);

		if (!_scene)
			return false;

		// Infer what is possible to load from this file.
		bool isOnlySkeleton = AssimpWrapper::isOnlySkeleton(_scene);

		if (isOnlySkeleton)
		{
			AssimpWrapper::loadOnlySkeleton(_scene, _scene->mRootNode, _skeleton, SMatrix::Identity);
			_skeleton._root = _skeleton.findBone(_scene->mRootNode->mName.C_Str());
		}
		else
		{
			bool isSkeletalModel = AssimpWrapper::containsRiggedMeshes(_scene);

			SMatrix rootTransform = AssimpWrapper::aiMatToSMat(_scene->mRootNode->mTransformation);
			_skeleton._globalInverseTransform = rootTransform.Invert();

			AssimpWrapper::loadBones(_scene, _scene->mRootNode, _skeleton);

			_skeleton.loadFromAssimp(_scene);
		}

		AssimpWrapper::loadAnimations(_scene, _anims);

		//_selectedAnim = nullptr;
		//if (_anims.size() > 0) _selectedAnim = &_anims[0];

		return true;
	}



	bool displayAiScene(const std::string& sName)
	{
		ImGui::Text("Assimp structure");

		if (ImGui::TreeNode("Node tree"))
		{
			printaiNode(_scene->mRootNode, _scene, _scene->mRootNode->mTransformation);

			ImGui::TreePop();
		}

		printSceneAnimations();

		ImGui::NewLine();
		ImGui::Text("Parsed assets");

		if (ImGui::TreeNode("Skeleton"))
		{
			printBoneHierarchy(_skeleton._root);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Textures"))
		{
			printTextures();
			ImGui::TreePop();
		}


		ImGui::NewLine();
		ImGui::Text("Commands");

		if (ImGui::Button("Export"))
		{
			_exporter.activate();
		}

		if(_exporter.isActive())
			_exporter.displayExportSettings();

		ImGui::InputInt("Animation to play: ", &_currentAnim);

		if (ImGui::Button("Close"))
		{
			return false;
		}

		return true;

		/*if (_selectedAnim)
		{
			ImGui::Begin("Animation editor");
			_animEditor.displayAnimation(_selectedAnim);
			ImGui::End();
		}*/

		//ImGui::SliderFloat("Playback speed", &_playbackSpeed, -1., 1.);
	}



	bool printaiNode(aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform)
	{
		std::string nodeName("Node: ");
		nodeName += node->mName.C_Str();

		if (ImGui::TreeNode(nodeName.c_str()))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.6, 0., 1., 1.));

			aiMatrix4x4 concatenatedTransform = node->mTransformation * parentTransform;
			SMatrix concatSMat = AssimpWrapper::aiMatToSMat(concatenatedTransform);

			if (ImGui::TreeNode("Concatenated transform: "))
			{
				displayTransform(concatSMat);

				SQuat squat = SQuat::CreateFromRotationMatrix(concatSMat);
				ImGui::InputFloat4("Quat: ", &squat.x, 3, ImGuiInputTextFlags_ReadOnly);

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

						std::string meshName("Mesh name and node index: ");
						meshName += mesh->mName.C_Str();
						meshName += " ";
						meshName += std::to_string(i);

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

			// This assumes files are exported with relative paths... which seems to be a big if...
			//std::string modelFolderPath = _path.substr(0, _path.find_last_of("/\\")) + "\\";
			std::string modelFolderPath = std::filesystem::path(_path).parent_path().string();
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
				const aiTexture* aiTex;

				if (_scene->mTextures)
				{
					aiTex = _scene->GetEmbeddedTexture(obtainedTexturePath.C_Str());
					loaded = (aiTex != nullptr);
				}

				if (loaded)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0., 1., 1., 1.));
					ImGui::Text("This texture is embedded!");
					ImGui::PopStyleColor();
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
	}



	void printBoneHierarchy(Bone* bone)
	{
		if (ImGui::TreeNode(bone->name.c_str()))
		{
			ImGui::Text("Index: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(bone->index).c_str());

			if (ImGui::TreeNode("Transformations"))
			{
				ImGui::TextColored(ImVec4(1., 0., 0., 1.), "Local matrix");
				displayTransform(bone->_localMatrix);

				//ImGui::TextColored(ImVec4(0., 1., 0., 1.), "Global matrix");
				//displayTransform(bone->_globalMatrix);

				ImGui::TextColored(ImVec4(0., 0., 1., 1.), "Inverse offset matrix");
				displayTransform(bone->_offsetMatrix);		

				ImGui::TreePop();
			}

			for (Bone* cBone : bone->offspring)
				printBoneHierarchy(cBone);

			ImGui::TreePop();
		}
	}



	void printTextures()
	{
		if (_scene->mTextures)
		{
			if (ImGui::TreeNode("Embedded"))
			{
				for (int i = 0; i < _scene->mNumTextures; ++i)
				{
					aiTexture* tex = _scene->mTextures[i];
					
					ImGui::Text("Name: ");
					ImGui::SameLine();
					ImGui::Text(tex->mFilename.C_Str());

					ImGui::Text("Width: %d; Height: %d;", tex->mWidth, tex->mHeight);

					ImGui::Text("Format hint: ");
					ImGui::SameLine();
					ImGui::Text(tex->achFormatHint);
				}

				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::Text("No embedded textures found");
		}
		

		if (ImGui::TreeNode("External"))
		{

			ImGui::TreePop();
		}
	}



	std::filesystem::path getPath() { return std::filesystem::path(_path); }

	int getCurrentAnim() { return _currentAnim; }

	float getPlaybackSpeed() { return _playbackSpeed; }

	const aiScene* getScene() { return _scene; }
};