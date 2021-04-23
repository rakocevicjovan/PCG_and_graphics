#pragma once
#include "Model.h"
#include "FileUtilities.h"
#include "AssetViews.h"
#include "Texture.h"
#include "FileBrowser.h"
#include "AnimationEditor.h"
#include "SkeletalModelInstance.h"
#include "SkeletonLoader.h"
#include "ShaderManager.h"
#include "MatLoader.h"
#include "AssimpGUI.h"

#include <entt/entt.hpp>

#include "TextureManager.h"

#include <cereal/cereal.hpp>
//#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>



/* Just adds complexity really...
namespace
{
	template<typename Archive, typename Asset, typename... Args>
	void serializeAsset(Archive& ar, const Asset& asset, Args...)
	{
		asset->save(ar);
	}
}*/


class AssImport
{
private:
	ResourceManager* _pResMan;
	AssetLedger* _pLedger;
	ShaderManager* _pShMan;
	//TextureCache* _pTexCache; @TODO merge with manager
	TextureManager _textureManager;

	std::string _path;
	std::string _sceneName;
	std::string _importPath = "C:\\Users\\metal\\Desktop\\AeonTest\\";

	Assimp::Importer _importer;
	const aiScene* _aiScene;

	// For 3d preview I need these... until a better system is in place at least
	std::unique_ptr<SkeletalModelInstance> _skModelInst;
	ID3D11Device* _device;

	// Things that might get loaded
	MatLoader::MatsAndTextureBlobs _matData;
	std::unique_ptr<SkeletalModel> _skModel;
	std::unique_ptr<Model> _model;
	std::shared_ptr<Skeleton> _skeleton;
	std::vector<Animation> _anims;

	//entt::registry _registry;

	// Import settings
	bool _hasOnlySkeleton, _hasSkeletalModel, _hasAnimations;
	bool _impSkeleton, _impSkModel, _impModel, _impAnims;
	bool _importConfigured;

	// Preview settings
	int _currentAnim;
	float _playbackSpeed;
	float _previewScale;

	std::vector<aiString> _extTexPaths;

public:



	bool loadAiScene(ID3D11Device* device, const std::string& path, UINT inFlags, 
		ResourceManager* resMan, ShaderManager* shMan)
	{
		_path = path;
		_sceneName = std::filesystem::path(path).filename().replace_extension("").string();
		_device = device;

		_pResMan = resMan;
		_pLedger = &resMan->_assetLedger;
		_pShMan = shMan;
		_textureManager = TextureManager(&resMan->_assetLedger, device);

		_importConfigured = false;

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

		_extTexPaths = AssimpWrapper::getExtTextureNames(_aiScene);

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
		_matData = MatLoader::LoadAllMaterials(_device, _aiScene, _path);

		if (_impSkModel)
		{
			_skModel = std::make_unique<SkeletalModel>();

			_skeleton = std::shared_ptr<Skeleton>(SkeletonLoader::loadSkeleton(_aiScene).release());

			_skModel->importFromAiScene(_device, _aiScene, _path, _matData._mats, _skeleton);

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
			_model->loadFromAiScene(_device, _aiScene, _path);

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
		ImGui::InputInt("Animation to play: ", &_currentAnim);
		ImGui::SliderFloat("Playback speed: ", &_playbackSpeed, 0.f, 1.f);

		ImGui::Text("Commands");


		if (ImGui::BeginPopup("Import target directory not found"))
		{
			ImGui::Text("Create the target import directory first.");
			ImGui::EndPopup();
		}

		if (ImGui::Button("Import as .aeon"))
		{
			if (std::filesystem::is_directory(_importPath))	// Add extra checks
			{
				persistAssets();
			}
			else
			{
				ImGui::OpenPopup("Import target directory not found");
				// Freak out
			}
		}

		if (ImGui::Button("Persist mat0 // Check .aeon"))
		{
			std::string matPath{ _importPath + _sceneName + "_mat_" + std::to_string(0) + ".aeon" };
			std::ofstream ofs(matPath, std::ios::binary);
			//cereal::BinaryOutputArchive boa(ofs);
			cereal::JSONOutputArchive output(ofs);
			//_matData._mats[0]->save(output, { 42, 69, 360, 420 });
		}

		if (ImGui::Button("Close"))
			return false;

		return true;
	}



	// Eeeeehhhh... weird way to do it.
	void persistAssets()
	{
		uint32_t skeletonID = persistSkeleton();
		std::vector<uint32_t> animIDs = persistAnims();
		persistTextures();
		std::vector<uint32_t> matIDs = persistMats();

		std::string mPath{ _importPath + _sceneName + ".aeon" };
		std::ofstream ofs(mPath, std::ios::binary);
		cereal::BinaryOutputArchive boa(ofs);

		if (_skModel.get())
		{
			_skModel->serialize(boa, matIDs, animIDs, skeletonID);
		}

		if (_model.get())
		{
			//_model->serialize();
		}

		_pLedger->save();
	}



	uint32_t persistSkeleton()
	{
		if (_skeleton.get())
		{
			std::string skeletonPath{ _importPath + _sceneName + "_skeleton" + ".aeon" };
			std::ofstream ofs(skeletonPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			_skeleton->serialize(boa);
			return _pLedger->insert(skeletonPath, ResType::SKELETON);
		}
		return 0;
	}



	std::vector<uint32_t> persistAnims()
	{
		std::vector<uint32_t> animIDs;
		for (UINT i = 0; i < _anims.size(); ++i)
		{
			std::string animName = _anims[i].getName();
			if (animName.size() == 0)	// Do this during import?
				animName = "anim_" + std::to_string(i);
			std::string animPath{ _importPath + animName + ".aeon" };
			std::ofstream ofs(animPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			_anims[i].serialize(boa);
			animIDs.push_back(_pLedger->insert(animPath, ResType::ANIMATION));
		}
		return animIDs;
	}



	void persistTextures()
	{
		for (auto& [name, blob, embedded] : _matData._blobs)
		{
			std::string texName = std::filesystem::path(name).filename().string();
			std::string texPath{ _importPath + texName };

			FileUtils::writeAllBytes(texPath, blob._data.get(), blob._size);
			//_textureManager.create(texPath.c_str(), )
			_pLedger->insert(texPath, ResType::TEXTURE);

			if (embedded)
				blob._data.release();

			blob._data.reset();
			blob._size = 0u;
		}
	}


	std::vector<AssetID> persistMats()
	{
		std::vector<uint32_t> matIDs;
		for (UINT i = 0; i < _matData._mats.size(); ++i)
		{
			// Get texture IDs
			auto& matMetaData = _matData._matMetaData[i];

			std::vector<AssetID> textureIDs;
			textureIDs.reserve(matMetaData._tempTexData.size());

			std::transform(matMetaData._tempTexData.begin(), matMetaData._tempTexData.end(), std::back_inserter(textureIDs),
				[&texMan = _textureManager](MatLoader::TempTexData& ttd)
				{
					return texMan.getID(ttd._path.c_str());
				});

			// Serialize
			std::string matPath{ _importPath + _sceneName + "_mat_" + std::to_string(i) + ".aeon" };
			std::ofstream ofs(matPath, std::ios::binary);	
			//cereal::BinaryOutputArchive boa(ofs);
			cereal::JSONOutputArchive output(ofs);

			_matData._mats[i]->save(output, textureIDs);

			matIDs.push_back(_pLedger->insert(matPath, ResType::MATERIAL));
		}
		return matIDs;
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



	std::filesystem::path getPath() { return std::filesystem::path(_path); }

	int getCurrentAnim() { return _currentAnim; }

	float getPlaybackSpeed() { return _playbackSpeed; }

	const aiScene* getScene() { return _aiScene; }

	~AssImport()
	{
		for (auto& texBlob : _matData._blobs)
		{
			if (texBlob.embedded)
				texBlob.blob._data.release();
		}
	}
};