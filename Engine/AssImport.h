#pragma once
#include "Model.h"
#include "FileUtilities.h"
#include "AssetViews.h"
#include "Texture.h"
#include "FileBrowser.h"
#include "AnimationEditor.h"
#include "SkeletalModelInstance.h"
#include "SkeletonImporter.h"
#include "ShaderManager.h"
#include "MatImporter.h"
#include "AssimpGUI.h"
#include "ModelImporter.h"
#include "AssetLedger.h"

// Structs representing serialized versions of different runtime types
#include "MaterialAsset.h"

#include <entt/entt.hpp>

#include <cereal/cereal.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>


class AssImport
{
private:
	AssetLedger* _pLedger;
	ShaderManager* _pShMan;

	std::string _srcPath;
	std::string _sceneName;
	std::string _destPath;

	std::vector<aiString> _extTexPaths;

	Assimp::Importer _importer;
	const aiScene* _aiScene;

	// For 3d preview I need these... until a better system is in place at least
	std::unique_ptr<SkeletalModelInstance> _skModelInst;
	ID3D11Device* _device;

	// Things that might get loaded
	MatImporter::MatsAndTextureBlobs _matData;
	std::unique_ptr<SkeletalModel> _skModel;
	std::unique_ptr<Model> _model;
	std::shared_ptr<Skeleton> _skeleton;
	std::vector<Animation> _anims;

	// Import settings
	bool _hasOnlySkeleton, _hasSkeletalModel, _hasAnimations;
	bool _impSkeleton, _impSkModel, _impModel, _impAnims;
	bool _importConfigured{ false };

	// Preview settings
	int _currentAnim{ 0u };
	int _numToDraw[3]{ 1, 1, 50 };
	float _playbackSpeed{ 1.f };
	float _previewScale{ 1.f };

public:

	bool loadAiScene(ID3D11Device* device, const char* importFrom, const char* importTo, AssetLedger& assetLedger, ShaderManager* shMan)
	{
		_srcPath = importFrom;
		_sceneName = std::filesystem::path(importFrom).filename().replace_extension("").string();
		_destPath = importTo;
		_device = device;

		_pLedger = &assetLedger;
		_pShMan = shMan;

		// Allow flag customization inhere
		unsigned int pFlags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_ConvertToLeftHanded |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_TransformUVCoords |
			aiProcess_PopulateArmatureData |
			aiProcess_SortByPType;

		// This doesn't work, allegedly because optimization flags are on
		//_importer.SetPropertyBool(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);	

		_aiScene = AssimpWrapper::loadScene(_importer, _srcPath, pFlags);

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

		_extTexPaths = AssimpWrapper::getExtTextureNames(_aiScene);

		return true;
	}


	void displayImportSettings()
	{
		if (ImGui::BeginChild("Import settings"))
		{
			if (_hasOnlySkeleton)
			{
				ImGui::Checkbox("Standalone skeleton", &_impSkeleton);
			}
			else
			{
				if (_hasSkeletalModel)
				{
					ImGui::Checkbox("Rigged model", &_impSkModel);
				}
				else
				{
					ImGui::Checkbox("Static model", &_impModel);
				}
			}

			if (_hasAnimations)
			{
				ImGui::Checkbox("Animations", &_impAnims);
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
			_skeleton = SkeletonImporter::loadStandalone(_aiScene);
		}

		if (_impAnims)
		{
			AssimpWrapper::loadAnimations(_aiScene, _anims);
		}

		// Pass these preloaded materials to meshes in either model type below
		_matData = MatImporter::ImportSceneMaterials(_device, _aiScene, _srcPath);

		if (_impSkModel)
		{
			_skeleton = std::shared_ptr<Skeleton>(SkeletonImporter::ImportSkeleton(_aiScene).release());

			_skModel = ModelImporter::ImportFromAiScene(_device, _aiScene, _srcPath, _matData._materials, _skeleton);

			// Skeletal model shouldn't even be pointing to animations tbh...
			for (Animation& anim : _anims)
				_skModel->_anims.push_back(&anim);

			// This code is here purely for presenting the loaded model
			for (SkeletalMesh& skmesh : _skModel->_meshes)
			{
				Material* skMat = skmesh.getMaterial();
				auto shPack = _pShMan->getShaderAuto(skmesh._vertSig, skMat);
				skMat->setVS(shPack->vs);
				skMat->setPS(shPack->ps);
			}

			_skModelInst = std::make_unique<SkeletalModelInstance>();
			_skModelInst->init(_device, _skModel.get());
		}

		if (_impModel)
		{
			//_importer.ApplyPostProcessing(aiProcess_PreTransformVertices);
			_model = std::make_unique<Model>();
			_model->loadFromAiScene(_device, _aiScene, _srcPath);

			for (Mesh& mesh : _model->_meshes)
			{
				auto shPack = _pShMan->getShaderAuto(mesh._vertSig, mesh._material.get());
				mesh._material->setVS(shPack->vs);
				mesh._material->setPS(shPack->ps);
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
		ImGui::Text("Assimp scene inspector");

		/* Not really needed now but can extend to do this too.
		if (ImGui::TreeNode("Meta data keys"))
		{
			for (int i = 0; i < _aiScene->mMetaData->mNumProperties; ++i)
			{
				ImGui::Text(_aiScene->mMetaData->mKeys[i].C_Str());
			}
			ImGui::TreePop();
		}*/

		if (ImGui::TreeNode("Node tree"))
		{
			AssimpGUI::printaiNode(_aiScene->mRootNode, _aiScene, _aiScene->mRootNode->mTransformation);
			ImGui::TreePop();
		}

		AssimpGUI::printSceneAnimations(_aiScene);

		if (ImGui::TreeNode("Textures"))
		{
			AssimpGUI::printAiSceneTextures(_aiScene, _extTexPaths);
			ImGui::TreePop();
		}
	}


	void displayParsedAssets()
	{
		ImGui::Text("Parsed asset inspector");

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

		ImGui::SliderFloat("Model scale: ", &_previewScale, .1f, 100.f);
		if (ImGui::InputInt("Animation to play: ", &_currentAnim))
		{
			if (_skModel && _currentAnim >= 0 && _currentAnim < _skModelInst->_animInstances.size())
			{
				_skModelInst->_animInstances[_currentAnim]._elapsed = 0.f;	// Prevent crashes 
			}
		}
		ImGui::SliderFloat("Playback speed: ", &_playbackSpeed, 0.f, 1.f);
		ImGui::InputInt3("Draw [#cols, #rows, spacing]", &_numToDraw[0]);

		ImGui::Text("Commands");


		if (ImGui::BeginPopup("Import target directory not found"))
		{
			ImGui::Text("Create the target import directory first.");
			ImGui::EndPopup();
		}

		if (ImGui::Button("Import as .aeon"))
		{
			if (std::filesystem::is_directory(_destPath))	// Add extra checks
			{
				persistAssets();
			}
			else
			{
				ImGui::OpenPopup("Import target directory not found");
				// Freak out
			}
		}

		if (ImGui::Button("Check .aeon"))
		{

		}

		if (ImGui::Button("Close"))
			return false;

		return true;
	}


	// Eeeeehhhh... weird way to do it.
	void persistAssets()
	{
		uint32_t skeletonID = persistSkeleton();
		std::vector<AssetID> animIDs = persistAnims();
		std::vector<AssetID> matIDs = persistMats();

		std::string mPath{ _destPath + _sceneName + ".aeon" };
		std::ofstream ofs(mPath, std::ios::binary);
		cereal::BinaryOutputArchive boa(ofs);

		if (_skModel.get())
		{
			_skModel->serialize(boa, matIDs, animIDs, skeletonID);
		}

		if (_model.get())
		{
			_model->serialize(boa, matIDs);
		}

		_pLedger->save();
	}


	uint32_t persistSkeleton()
	{
		if (_skeleton.get())
		{
			std::string skeletonPath{ _destPath + _sceneName + "_skeleton" + ".aeon" };
			std::ofstream ofs(skeletonPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			//cereal::JSONOutputArchive boa(ofs);
			_skeleton->serialize(boa);
			return _pLedger->insert(skeletonPath.c_str(), ResType::SKELETON);
		}
		return 0;
	}


	std::vector<uint32_t> persistAnims()
	{
		std::vector<uint32_t> animIDs;
		for (UINT i = 0; i < _anims.size(); ++i)
		{
			std::string animName = (_anims[i].getName().size() == 0 ? std::to_string(i) : _anims[i].getName()) + "_anim";
			std::string animPath{ _destPath + animName + ".aeon" };

			std::ofstream ofs(animPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);

			_anims[i].serialize(boa);
			animIDs.push_back(_pLedger->insert(animPath.c_str(), ResType::ANIMATION));
		}
		return animIDs;
	}


	AssetID persistTexture(const std::string& name, const std::pair<std::shared_ptr<Texture>, MatImporter::RawTextureData>& texture)
	{
		auto& [tex, rawTexData] = texture;

		std::string texName = std::filesystem::path(name).filename().string();
		std::string texPath{ _destPath + texName };

		auto [exists, assetID] = _pLedger->getOrInsert(texPath.c_str(), ResType::TEXTURE);

		if (!exists)
		{
			FileUtils::writeAllBytes(texPath, rawTexData.blob.data(), rawTexData.blob.size());
		}

		return assetID;
	}


	std::vector<AssetID> persistMats()
	{
		std::vector<uint32_t> matIDs;

		for (auto i = 0; i < _matData._materials.size(); ++i)
		{
			// Get texture IDs
			auto& matMetaData = _matData._materialData[i];
			
			MaterialAsset matAsset;

			// Hardcoded for now, fix
			matAsset._opaque = true;

			matAsset._textures.reserve(matMetaData._tempTexData.size());

			std::transform(matMetaData._tempTexData.begin(), matMetaData._tempTexData.end(), std::back_inserter(matAsset._textures),
				[&](MatImporter::TexturePathAndMetadata& texPathAndMetaData)
				{
					AssetID textureID = persistTexture(texPathAndMetaData.path, _matData._textures.at(texPathAndMetaData.path));
					return MaterialAsset::AssetMaterialTexture{texPathAndMetaData.metaData, textureID};
				});

			// Serialize
			std::string matPath{ _destPath + _sceneName + "_mat_" + std::to_string(i) + ".aeon" };
			std::ofstream ofs(matPath, std::ios::binary);	
			cereal::JSONOutputArchive output(ofs);	//cereal::BinaryOutputArchive boa(ofs);

			matAsset.serialize(output);

			matIDs.push_back(_pLedger->insert(matPath.c_str(), ResType::MATERIAL));
		}
		return matIDs;
	}


	void draw(ID3D11DeviceContext* context, float dTime)
	{
		auto [columns, rows, spacing] = _numToDraw;
		float numDrawn = columns * rows;
		float fakeDTime = dTime / numDrawn;

		for (int i = 0; i < numDrawn; ++i)
		{
			SVec3 offset = SVec3(i % columns, 0, i / columns) * spacing;

			if (_skModel)
			{
				_skModelInst->update(fakeDTime * _playbackSpeed, _currentAnim);
				Math::SetTranslation(_skModelInst->_transform, offset);
				Math::SetScale(_skModelInst->_transform, SVec3(_previewScale));
				_skModelInst->draw(context);
			}

			if (_model)
			{
				Math::SetTranslation(_model->_transform, offset);
				Math::SetScale(_model->_transform, SVec3(_previewScale));

				for (Mesh& r : _model->_meshes)
				{
					r._worldSpaceTransform = _model->_transform * r._parentSpaceTransform;
					r.draw(context);
				}
			}
		}
		
	}


	std::filesystem::path getPath() { return std::filesystem::path(_srcPath); }

	int getCurrentAnim() { return _currentAnim; }

	float getPlaybackSpeed() { return _playbackSpeed; }

	const aiScene* getScene() { return _aiScene; }

	~AssImport()
	{
		for (auto& texture : _matData._textures)
		{
			texture.second.second.freeMemory();
		}
	}
};