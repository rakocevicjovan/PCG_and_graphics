#pragma once
#include "Model.h"
#include "FileUtilities.h"
#include "AssetViews.h"
#include "Texture.h"
#include "FileBrowser.h"
#include "AnimationEditor.h"
#include "SkeletalModelInstance.h"
#include "Model.h"
#include "SkeletonLoader.h"
#include "ShaderManager.h"
#include "MatLoader.h"



class AssImport
{
private:
	ResourceManager* _pResMan;
	AssetLedger* _pLedger;
	ShaderManager* _pShMan;
	TextureCache* _pTexCache;

	std::string _path;
	std::string _sceneName;
	std::string _importPath = "C:\\Users\\Senpai\\Desktop\\AeonTest\\";

	Assimp::Importer _importer;
	const aiScene* _aiScene;

	// For 3d preview I need these... until a better system is in place at least
	std::unique_ptr<SkeletalModelInstance> _skModelInst;
	ID3D11Device* _device;

	// Things that might get loaded
	std::unique_ptr<SkeletalModel> _skModel;
	std::unique_ptr<Model> _model;
	std::unique_ptr<Skeleton> _skeleton;
	std::vector<Animation> _anims;
	std::vector<Material> _mats;
	std::vector<Texture> _textures;


	bool _hasOnlySkeleton, _hasSkeletalModel, _hasAnimations;
	bool _impSkeleton, _impSkModel, _impModel, _impAnims;
	bool _importConfigured;

	// Put this into animEditor
	int _currentAnim;
	float _playbackSpeed;
	float _previewScale;

	std::vector<aiString> _externalTextures;

public:



	bool loadAiScene(ID3D11Device* device, const std::string& path, UINT inFlags, 
		ResourceManager* resMan, ShaderManager* shMan)
	{
		_pResMan = resMan;
		_pLedger = &resMan->_assetLedger;
		_pShMan = shMan;

		_importConfigured = false;

		_path = path;
		_sceneName = std::filesystem::path(path).filename().replace_extension("").string();
		_device = device;

		_previewScale = 1.f;
		_currentAnim = 0;
		_playbackSpeed = 1.f;

		unsigned int pFlags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_ConvertToLeftHanded |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_TransformUVCoords |
			aiProcess_PopulateArmatureData;

		// This doesn't work, allegedly because optimization flags are on
		//_importer.SetPropertyBool(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);	

		_aiScene = AssimpWrapper::loadScene(_importer, path, pFlags);

		if (!_aiScene)
			return false;

		_hasOnlySkeleton = AssimpWrapper::isOnlySkeleton(_aiScene);
		_hasSkeletalModel = AssimpWrapper::containsRiggedMeshes(_aiScene);
		_hasAnimations = _aiScene->HasAnimations();

		// Checkbox defaults, usual case is to load everything
		_impSkeleton = _hasOnlySkeleton;
		_impSkModel = _hasSkeletalModel;
		_impAnims = _hasAnimations;
		_impModel = !_hasOnlySkeleton && !_hasSkeletalModel;

		_externalTextures = AssimpWrapper::getExtTextureNames(_aiScene);

		return true;
	}



	void displayImportSettings()
	{
		if (ImGui::BeginChild("Import settings"))
		{
			if (_hasOnlySkeleton)
			{
				ImGui::Text("Scene contains a standalone skeleton.");
				ImGui::Checkbox("Load skeleton", &_impSkeleton);
			}
			else
			{
				if (_hasSkeletalModel)
				{
					ImGui::Text("Scene contains a rigged model.");
					ImGui::Checkbox("Load rigged model", &_impSkModel);
				}
				else
				{
					ImGui::Text("Scene contains a static model.");
					ImGui::Checkbox("Load static model", &_impModel);
				}
			}

			if (_hasAnimations)
			{
				ImGui::Text("Scene contains animations");
				ImGui::Checkbox("Load animations", &_impAnims);
			}

			if (ImGui::Button("Import selected"))
			{
				importSelectedAssets();
				_importConfigured = true;
			}
		}
		ImGui::EndChild();
	}



	void importSelectedAssets()
	{
		if (_impSkeleton)
		{
			_skeleton = SkeletonLoader::loadStandalone(_aiScene);
		}

		if (_impAnims)
		{
			AssimpWrapper::loadAnimations(_aiScene, _anims);
		}

		// Pass these preloaded materials to meshes in either model type below
		_mats = MatLoader::LoadAllMaterials(_aiScene, _path, _pTexCache);

		if (_impSkModel)
		{
			_skModel = std::make_unique<SkeletalModel>();

			_skeleton = SkeletonLoader::loadSkeleton(_aiScene);
			_skModel->_skeleton = _skeleton.get();

			_skModel->loadFromAiScene(_device, _aiScene, _path);

			// Skeletal model shouldn't even be pointing to animations tbh...
			for (Animation& anim : _anims)
				_skModel->_anims.push_back(&anim);

			// This code is here purely for presenting the loaded model
			for (SkeletalMesh& skmesh : _skModel->_meshes)
			{
				auto shPack = _pShMan->getShaderAuto(skmesh._vertSig, &skmesh._baseMaterial);
				skmesh._baseMaterial.setVS(shPack->vs);
				skmesh._baseMaterial.setPS(shPack->ps);
			}

			_skModelInst = std::make_unique<SkeletalModelInstance>();
			_skModelInst->init(_device, _skModel.get());
		}

		if (_impModel)
		{
			_importer.ApplyPostProcessing(aiProcess_PreTransformVertices);
			_model = std::make_unique<Model>();
			_model->loadFromAiScene(_device, _aiScene, _path);

			for (Mesh& mesh : _model->_meshes)
			{
				auto shPack = _pShMan->getShaderAuto(mesh._vertSig, &mesh._baseMaterial);
				mesh._baseMaterial.setVS(shPack->vs);
				mesh._baseMaterial.setPS(shPack->ps);
			}
		}
	}



	bool displayPreview(const std::string& sName)
	{
		if (!_importConfigured)
		{
			displayImportSettings();
			return true;
		}

		displayAiScene();

		ImGui::NewLine();

		displayParsedAssets();

		ImGui::NewLine();

		return displayCommands();
	}



	void displayAiScene()
	{
		ImGui::Text("Assimp scene");

		if (ImGui::TreeNode("Node tree"))
		{
			printaiNode(_aiScene->mRootNode, _aiScene, _aiScene->mRootNode->mTransformation);
			ImGui::TreePop();
		}

		printSceneAnimations();

		if (ImGui::TreeNode("Textures"))
		{
			printAiSceneTextures();
			ImGui::TreePop();
		}
	}



	void displayParsedAssets()
	{
		ImGui::Text("Parsed assets");

		if (_skeleton.get())
			AssetViews::printSkeleton(_skeleton.get());
		
		if (_skModel.get())
			AssetViews::printSkModel(_skModel.get());

		if (_model.get())
			AssetViews::printModel(_model.get());
	}



	bool displayCommands()
	{
		ImGui::Text("Preview settings");

		ImGui::SliderFloat("Model scale: (tbd)", &_previewScale, .1f, 100.f);
		ImGui::InputInt("Animation to play: ", &_currentAnim);
		ImGui::SliderFloat("Playback speed: ", &_playbackSpeed, 0.f, 1.f);

		ImGui::Text("Commands");

		if (ImGui::Button("Import as .aeon"))
		{
			if (std::filesystem::is_directory(_importPath))	// Add extra checks
			{
				persistAssets();
			}
			else
			{
				// Freak out
			}
		}

		if (ImGui::Button("Close"))
			return false;

		return true;
	}



	// Eeeeehhhh... weird way to do it.
	void persistAssets()
	{
		UINT skeletonID;
		std::vector<UINT> animIDs;
		std::vector<UINT> texIDs;
		std::vector<UINT> matIDs;

		if (_skeleton.get())
		{
			std::string skeletonPath{ _importPath + "skelly" + ".aeon" };
			std::ofstream ofs(skeletonPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			_skeleton->serialize(boa);
			skeletonID = _pLedger->add("", skeletonPath, ResType::SKELETON);
		}

		for (UINT i = 0; i < _anims.size(); ++i)
		{
			std::string animName = _anims[i].getName();
			if (animName.size() == 0)	// Do this during import?
				animName = "anim_" + std::to_string(i);
			std::string animPath{ _importPath + animName + ".aeon" };
			std::ofstream ofs(animPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			_anims[i].serialize(boa);
			animIDs.push_back(_pLedger->add("", animPath, ResType::ANIMATION));
		}

		for (UINT i = 0; i < _mats.size(); ++i)
		{
			std::string matPath{ _importPath + "mat_" + std::to_string(i)+ ".aeon" };
			std::ofstream ofs(matPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			_mats[i].save(boa, {});
			matIDs.push_back(_pLedger->add("", matPath, ResType::MATERIAL));
		}

		std::string modPath{ _importPath + _sceneName };
		std::string mPath{ _importPath + _sceneName + ".aeon" };
		std::ofstream ofs(mPath, std::ios::binary);
		cereal::BinaryOutputArchive boa(ofs);

		if (_skModel.get())
		{
			_skModel->serialize(boa, matIDs, animIDs, skeletonID);
		}

		if (_model.get())
		{
			//_model->serialize(ofs, )
		}

		_pLedger->save();
	}



	void draw(ID3D11DeviceContext* context, float dTime)
	{
		if (_skModel)
		{
			_skModelInst->update(dTime * _playbackSpeed, _currentAnim);
			Math::SetScale(_skModelInst->_transform, SVec3(_previewScale));
			_skModelInst->draw(context);
		}
		
		if (_model)
		{
			for (Mesh& r : _model->_meshes)
			{
				Math::SetScale(r._transform, SVec3(_previewScale));
				r.draw(context);
			}
		}
	}



	bool printaiNode(aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform)
	{
		std::string nodeName("Node: ");
		nodeName += node->mName.C_Str();

		if (ImGui::TreeNode(nodeName.c_str()))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.6, 0., 1., 1.));

			aiMatrix4x4 concatenatedTransform = node->mTransformation * parentTransform;

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();

				ImGui::TextColored(ImVec4(1., 0., 0., 1.), "Local matrix");

				SMatrix concatSMat = AssimpWrapper::aiMatToSMat(concatenatedTransform);
				GuiBlocks::displayTransform(concatSMat);

				SQuat squat = SQuat::CreateFromRotationMatrix(concatSMat);
				ImGui::InputFloat4("Quat: ", &squat.x, 3, ImGuiInputTextFlags_ReadOnly);

				ImGui::EndTooltip();
			}


			ImGui::Text("Mesh count: %d", node->mNumMeshes);

			if (node->mNumMeshes > 0)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0., 1., 0., 1.));
				if (ImGui::TreeNode("Meshes"))
				{
					for (unsigned int i = 0; i < node->mNumMeshes; i++)
					{
						aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

						ImGui::PushID(i);
						if (ImGui::TreeNode("Mesh name and node index: %s (%d)", mesh->mName.data, i))
						{
							printAiMesh(mesh, concatenatedTransform);
							ImGui::TreePop();
						}
						ImGui::PopID();

					}
					ImGui::TreePop();
				}
				ImGui::PopStyleColor();
			}


			ImGui::Separator();

			ImGui::Text("Child count: %d", node->mNumChildren);

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
		// Just because they are there doesn't mean they are useful, sadly... 
		// It's infuriating how many issues need to be ironed out during import, like removing duplicates
		UINT numUVChannels = mesh->GetNumUVChannels();
		UINT* numUVComponents = mesh->mNumUVComponents;

		ImGui::Text("Nr. of UV channels: %d", numUVChannels);
		ImGui::Text("Nr. of UV components per channel: ");

		ImGui::Indent();
		for (int i = 0; i < numUVChannels; i++)
			ImGui::Text("%d: %d ", i, numUVComponents[i]);
		ImGui::Unindent();

		ImGui::NewLine();

		ImGui::Text("Vertex count: %d", mesh->mNumVertices);
		ImGui::Text("Index count: %d", mesh->mNumFaces * 3);	// I always use assimp's triangulate flag
		ImGui::Text("Face count: %d", mesh->mNumFaces);
		ImGui::Text("Has (bi)tangents: %d", mesh->HasTangentsAndBitangents());

		ImGui::NewLine();

		printAiMaterial(mesh);
	}



	void printAiMaterial(aiMesh* mesh)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.4, .5, .7, 1.));
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* aiMat = _aiScene->mMaterials[mesh->mMaterialIndex];

			ImGui::Text("Material: %s", aiMat->GetName().C_Str());

			// Diffuse maps
			printMaterialTextures(aiMat, aiTextureType_DIFFUSE, "texture_diffuse", DIFFUSE);

			// Normal maps
			printMaterialTextures(aiMat, aiTextureType_NORMALS, "texture_normal", NORMAL);

			// Specular maps
			printMaterialTextures(aiMat, aiTextureType_SPECULAR, "texture_specular", SPECULAR);

			// Shininess maps
			printMaterialTextures(aiMat, aiTextureType_SHININESS, "texture_shininess", SHININESS);

			// Opacity maps
			printMaterialTextures(aiMat, aiTextureType_OPACITY, "texture_opacity", OPACITY);

			// Displacement maps
			printMaterialTextures(aiMat, aiTextureType_DISPLACEMENT, "texture_disp", DPCM);

			// Ambient occlusion maps
			printMaterialTextures(aiMat, aiTextureType_AMBIENT, "texture_AO", AMB_OCCLUSION);

			// Other maps
			printMaterialTextures(aiMat, aiTextureType_UNKNOWN, "texture_other", OTHER);

			// Weird properties... that I never really saw trigger
			printMaterialTextures(aiMat, aiTextureType_NONE, "texture_property", OTHER);

			ImGui::Unindent();
		}
		else
		{
			ImGui::Text("Material: none");
		}

		ImGui::PopStyleColor();
	}



	void printMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, TextureRole role)
	{
		UINT numThisType = mat->GetTextureCount(type);
		typeName += " count: " + std::to_string(numThisType);

		ImGui::Text(typeName.c_str());

		ImGui::Indent();

		//iterate all textures of relevant related to the material
		for (UINT i = 0; i < numThisType; ++i)
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
				const aiTexture* aiTex = nullptr;

				if (_aiScene->mTextures)
					aiTex = _aiScene->GetEmbeddedTexture(obtainedTexturePath.C_Str());

				if (aiTex)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0., 1., 1., 1.));
					ImGui::Text("This texture is embedded!");
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::Text("Path: %s", texPath.c_str());
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1., 0., 0., 1.));
					ImGui::Text(" ( WARNING: NOT FOUND! )");
					ImGui::PopStyleColor();

					// Try to apply the fix for absolute paths to textures...
					std::filesystem::directory_entry artistPls;
					if (FileUtils::findFile(modelFolderPath, texName, artistPls))
					{
						ImGui::Indent();
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.93, 0.82, 0.01, 1.));
						ImGui::Text("Proposed path: %s", artistPls.path().string().c_str());
						ImGui::PopStyleColor();
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
				printAiAnimation(_aiScene->mAnimations[i]);

			ImGui::TreePop();
		}
	}



	void printAiAnimation(aiAnimation* sceneAnimation)
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
					printAiAnimationTrack(channel);
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}



	void printAiAnimationTrack(aiNodeAnim* channel)
	{
		ImGui::Text("Num scaling keys: %d", channel->mNumScalingKeys);
		ImGui::Text("Num rotation keys: %d", channel->mNumRotationKeys);
		ImGui::Text("Num position keys: %d", channel->mNumPositionKeys);
	}



	void printAiSceneTextures()
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