#pragma once
#include "Model.h"
#include "FileUtilities.h"
#include "GuiBlocks.h"
#include "Texture.h"
#include "FileBrowser.h"
#include "Exporter.h"
#include "AnimationEditor.h"
#include "SkeletalModelInstance.h"



class AssimpPreview
{
private:

	std::string _path;
	Assimp::Importer _importer;
	const aiScene* _aiScene;

	// For preview
	ID3D11Device* _device;
	std::unique_ptr<SkeletalModel> _skModel;
	std::unique_ptr<SkeletalModelInstance> _skModelInst;
	Material* _skelAnimMat;
	PointLight* _pLight;

	bool isOnlySkeleton;
	bool isSkeletalModel;
	bool containsAnimations;

	Exporter _exporter;

	// Put this into animEditor
	int _currentAnim;
	float _playbackSpeed;
	//AnimationEditor _animEditor;	//@TODO sequencer and all that, not a priority yet
	//Animation* _selectedAnim;

	// It's unnecessary to parse this per frame so I do it once and keep it
	Skeleton _skeleton;
	std::vector<Animation> _anims;
	std::vector<aiString> _externalTextures;
	

public:



	bool loadAiScene(ID3D11Device* device, const std::string& path, UINT inFlags, Material* defMat, PointLight* pLight)
	{
		_currentAnim = 0;
		_playbackSpeed = 1;

		_path = path;
		_device = device;
		_skelAnimMat = defMat;
		_pLight = pLight;

		unsigned int pFlags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_ConvertToLeftHanded;

		_importer.SetPropertyBool(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);	// This doesn't work...

		_aiScene = AssimpWrapper::loadScene(_importer, path, pFlags);

		if (!_aiScene)
			return false;

		isOnlySkeleton = AssimpWrapper::isOnlySkeleton(_aiScene);

		if (isOnlySkeleton)
		{
			AssimpWrapper::loadOnlySkeleton(_aiScene, _aiScene->mRootNode, _skeleton, SMatrix::Identity);
			_skeleton._root = _skeleton.findBone(_aiScene->mRootNode->mName.C_Str());
		}
		else
		{
			isSkeletalModel = AssimpWrapper::containsRiggedMeshes(_aiScene);

			SMatrix rootTransform = AssimpWrapper::aiMatToSMat(_aiScene->mRootNode->mTransformation);
			_skeleton._globalInverseTransform = rootTransform.Invert();

			AssimpWrapper::loadBones(_aiScene, _aiScene->mRootNode, _skeleton);

			_skeleton.loadFromAssimp(_aiScene);
		}

		AssimpWrapper::loadAnimations(_aiScene, _anims);
		containsAnimations = (_anims.size() > 0);

		_externalTextures = AssimpWrapper::loadExternalTextures(_aiScene);

		return true;
	}



	bool displayAiScene(const std::string& sName)
	{
		ImGui::Text("Assimp structures");

		if (ImGui::TreeNode("Node tree"))
		{
			printaiNode(_aiScene->mRootNode, _aiScene, _aiScene->mRootNode->mTransformation);

			ImGui::TreePop();
		}

		printSceneAnimations();

		if (ImGui::TreeNode("Textures"))
		{
			printTextures();
			ImGui::TreePop();
		}


		ImGui::NewLine();
		ImGui::Text("Parsed assets");

		if (ImGui::TreeNode("Skeleton"))
		{
			printBoneHierarchy(_skeleton._root);
			ImGui::TreePop();
		}

		ImGui::NewLine();

		return displayCommands();
	}



	bool displayCommands()
	{		
		ImGui::Text("Commands");

		if (ImGui::Button("Load as skeletal model"))
		{
			_skModel.reset();
			_skModel = std::make_unique<SkeletalModel>();
			_skModel->loadFromScene(_device, _aiScene);

			for (SkeletalMesh& skmesh : _skModel->_meshes)
			{
				skmesh._baseMaterial.setVS(_skelAnimMat->getVS());
				skmesh._baseMaterial.setPS(_skelAnimMat->getPS());
				skmesh._baseMaterial.pLight = _pLight;
			}

			_skModelInst.reset();
			_skModelInst = std::make_unique<SkeletalModelInstance>();
			_skModelInst->init(_device, _skModel.get());
		}
		sizeof(BonedVert3D);
		if (ImGui::Button("Export"))
			_exporter.activate();

		if (_exporter.isActive())
		{
			if (_exporter.displayExportSettings())
			{
				//Gather settings, process them
				if (!_skModel.get())
					return true;

				MemChunk chunk = Serializer::serializeSkeletalModel(*_skModel.get());
				_exporter.exportAsset(chunk);
			}
		}

		ImGui::InputInt("Animation to play: ", &_currentAnim);

		if (ImGui::Button("Close"))
			return false;

		return true;
	}



	void draw(ID3D11DeviceContext* context, float dTime)
	{
		if (!_skModel)
			return;

		_skModelInst->update(dTime * _playbackSpeed, _currentAnim);
		_skModelInst->draw(context);
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

		ImGui::Text("Nr. of UV channels: %d", numUVChannels);

		ImGui::Text("Nr. of UV components per channel: ");
		ImGui::Indent();
		for (int i = 0; i < numUVChannels; i++)
		{
			ImGui::Text("##%d", numUVComponents[i]);
		}
		ImGui::Unindent();

		ImGui::EndGroup();
		ImGui::NewLine();

		UINT indexCount = 0u;

		indexCount = mesh->mNumFaces * 3;	// Much quicker approximation, if assimp triangulation worked

		/*for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			//populate indices from faces
			for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
				++indexCount;	//_indices.push_back(face.mIndices[j]);
		}*/

		ImGui::BeginGroup();
		ImGui::Text("Vertex count: %d", mesh->mNumVertices);
		ImGui::Text("Index count: %d", indexCount);
		ImGui::Text("Face count: %d", mesh->mNumFaces);
		ImGui::Text("Has tangents and bitangents: %d", mesh->HasTangentsAndBitangents());
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
			aiMaterial* material = _aiScene->mMaterials[mesh->mMaterialIndex];

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

			bool texFound = FileUtils::fileExists(texPath);
			
			if (texFound)
			{
				ImGui::Text("Path: %s", texPath.c_str());
			}
			else
			{
				const aiTexture* aiTex;

				if (_aiScene->mTextures)
				{
					aiTex = _aiScene->GetEmbeddedTexture(obtainedTexturePath.C_Str());
					texFound = (aiTex != nullptr);
				}

				if (texFound)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0., 1., 1., 1.));
					ImGui::Text("This texture is embedded!");
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::Text("Path: %s", texPath.c_str());
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
		if (!_aiScene->HasAnimations())
			return;

		if(ImGui::TreeNode("Animation list"))
		{
			for (int i = 0; i < _aiScene->mNumAnimations; ++i)
				printAnimation(_aiScene->mAnimations[i]);

			ImGui::TreePop();
		}
	}



	void printAnimation(aiAnimation* sceneAnimation)
	{
		int numChannels = sceneAnimation->mNumChannels;

		if (ImGui::TreeNode(sceneAnimation->mName.C_Str()))
		{
			ImGui::Text("Num channels: %d", numChannels);

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
		ImGui::Text("Num scaling keys: %d", channel->mNumScalingKeys);
		ImGui::Text("Num rotation keys: %d", channel->mNumRotationKeys);
		ImGui::Text("Num position keys: %d", channel->mNumPositionKeys);
	}



	void printBoneHierarchy(Bone* bone)
	{
		if (!bone)
		{
			ImGui::Text("Bone is nullptr");
			return;
		}

		if (ImGui::TreeNode(bone->name.c_str()))
		{
			ImGui::Text("Index: %d", bone->index);

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
		if (_aiScene->mTextures)
		{
			if (ImGui::TreeNode("Embedded"))
			{
				for (int i = 0; i < _aiScene->mNumTextures; ++i)
				{
					const aiTexture* tex = _aiScene->mTextures[i];	
					ImGui::Text("Name: %s", tex->mFilename.C_Str());
					ImGui::Text("Width: %d; Height: %d;", tex->mWidth, tex->mHeight);
					ImGui::Text("Format hint: %s", tex->achFormatHint);
				}

				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::Text("No embedded textures found");
		}
		
		if (_externalTextures.size() > 0)
		{
			if (ImGui::TreeNode("External"))
			{
				for (int i = 0; i < _externalTextures.size(); ++i)
					ImGui::Text(_externalTextures[i].C_Str());

				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::Text("No external textures found");
		}
		
	}



	std::filesystem::path getPath() { return std::filesystem::path(_path); }

	int getCurrentAnim() { return _currentAnim; }

	float getPlaybackSpeed() { return _playbackSpeed; }

	const aiScene* getScene() { return _aiScene; }
};