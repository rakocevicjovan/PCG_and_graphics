#include "pch.h"
#include "AssImport.h"
#include "AssetViews.h"
#include "AssimpGUI.h"
#include "AssetLedger.h"
#include "FileUtilities.h"


bool AssImport::importAiScene(ID3D11Device* device, const char* importFrom, const char* importTo, AssetLedger& assetLedger, ShaderManager* shMan)
{
	_srcPath = importFrom;
	_sceneName = std::filesystem::path(importFrom).filename().replace_extension("").string();
	_destPath = importTo;
	_device = device;

	_pLedger = &assetLedger;
	_pShMan = shMan;

	// Allow flag customization inhere
	uint32_t pFlags = ModelImporter::DEFAULT_IMPORT_FLAGS;

	_aiScene = AssimpWrapper::loadScene(_importer, _srcPath, pFlags);

	if (!_aiScene)
	{
		return false;
	}

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


void AssImport::displayImportSettings()
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


void AssImport::importSelectedAssets()
{
	if (_impSkeleton)
	{
		_skeleton = SkeletonImporter::ImportStandalone(_aiScene);
	}

	if (_impAnims)
	{
		AssimpWrapper::ImportAnimations(_aiScene, _anims);
	}

	// Pass these preloaded materials to meshes in either model type below
	_matData = MatImporter::ImportSceneMaterials(_device, _aiScene, _srcPath);


	std::vector<Mesh>* importedMeshes{};

	if (_impSkModel)
	{
		_skeleton = SkeletonImporter::ImportSkeleton(_aiScene);
		_skModelData = ModelImporter::ImportSkModelFromAiScene(_device, _aiScene, _srcPath, _matData._materials, _skeleton);

		auto& skModel = _skModelData.model;

		// Skeletal model shouldn't even be pointing to animations tbh...
		for (Animation& anim : _anims)
		{
			skModel->_anims.push_back(std::make_shared<Animation>(anim));
		}

		importedMeshes = &(_skModelData.model->_meshes);

		_skModelInst = std::make_unique<SkeletalModelInstance>();
		_skModelInst->init(_device, skModel.get());
	}

	if (_impModel)
	{
		_modelData = ModelImporter::ImportModelFromAiScene(_device, _aiScene, _srcPath, _matData._materials);

		importedMeshes = &(_skModelData.model->_meshes);
	}

	// File might not have any meshes
	if (importedMeshes)
	{
		for (auto i = 0; i < importedMeshes->size(); ++i)
		{
			auto& mesh = importedMeshes->at(i);
			const auto& matIndex = _impModel ? _modelData.meshMaterialMapping[i] : _skModelData.meshMaterialMapping[i];

			Material* mat = mesh.getMaterial();
			auto shPack = _pShMan->getBestFit(mesh._vertSig, mat);

			mat->setVS(shPack->vs);
			mat->setPS(shPack->ps);

			// This will cause shaders to be generated and tracked by the ledger regardless of whether the model is imported.
			auto vsPathStr = std::string(shPack->vs->_path.begin(), shPack->vs->_path.end());
			auto psPathStr = std::string(shPack->ps->_path.begin(), shPack->ps->_path.end());

			auto vsAssetID = _pLedger->getOrInsert(AssetMetaData{ vsPathStr, {}, EAssetType::SHADER });
			auto psAssetID = _pLedger->getOrInsert(AssetMetaData{ psPathStr, {}, EAssetType::SHADER });

			_shadersPerMaterial[matIndex] = { vsAssetID, psAssetID, 0, 0, 0, 0 };
		}
	}
}


bool AssImport::displayPreview()
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


void AssImport::displayAiScene()
{
	ImGui::Text("Assimp scene inspector");

	/* Not really needed now but can be useful
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


void AssImport::displayParsedAssets()
{
	ImGui::Text("Parsed asset inspector");

	if (_skeleton.get() && ! _skModelData)
		AssetViews::PrintSkeleton(_skeleton.get());

	if (_skModelData)
		AssetViews::PrintSkModel(_skModelData.model.get());

	if (_modelData)
		AssetViews::PrintModel(_modelData.model.get());
}


bool AssImport::displayCommands()
{
	ImGui::Text("Preview settings");

	ImGui::SliderFloat("Model scale: ", &_previewScale, .1f, 100.f);
	if (ImGui::InputInt("Animation to play: ", &_currentAnim))
	{
		if (_skModelData && _currentAnim >= 0 && _currentAnim < _skModelInst->_animInstances.size())
		{
			_skModelInst->_animInstances[_currentAnim].restart();	// Prevent crashes when setting a new animation.
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

	if (ImGui::Button("Verify .aeon worked"))
	{
		/*
		if (_skModelData)
		{
			_skModelData.model.reset();

			std::string importPath{ _destPath + _sceneName + ".aeon" };
			auto skModelAsset = AssetHelpers::DeserializeFromFile<SkModelAsset>(importPath.c_str());

			_skModelData.model = ModelLoader::LoadSkModelFromAsset(skModelAsset, *_pLedger);

			for (Mesh& skMesh : _skModelData.model->_meshes)
			{
				Material* skMat = skMesh.getMaterial();
				auto shPack = _pShMan->getShaderByData(skMesh._vertSig, skMat);
				skMat->setVS(shPack->vs);
				skMat->setPS(shPack->ps);

				skMesh.setupMesh(_device);
			}

			_skModelInst = std::make_unique<SkeletalModelInstance>();
			_skModelInst->init(_device, _skModelData.model.get());
		}
		*/

	}

	return !ImGui::Button("Close");
}


void AssImport::persistAssets()
{
	AssetID skeletonID = persistSkeleton();
	std::vector<AssetID> animIDs = persistAnims();
	std::vector<AssetID> matIDs = persistMats();

	std::string importPath{ _destPath + _sceneName + ".aeon" };

	AssetID importedID{};

	if (_modelData)
	{
		auto modelAsset = makeModelAsset(*_modelData.model, matIDs);
		importedID = persistBinary(*modelAsset, { importPath, matIDs, EAssetType::MODEL }, *_pLedger);
	}

	if (_skModelData)
	{
		auto skModelAsset = makeSkModelAsset(*_skModelData.model, matIDs, animIDs, skeletonID);

		auto allDeps = std::move(matIDs);
		//allDeps.insert(allDeps.end(), matIDs.begin(), matIDs.end());
		allDeps.push_back(skeletonID);
		importedID = persistBinary(*skModelAsset, { importPath, allDeps, EAssetType::SK_MODEL }, *_pLedger);
	}
}


AssetID AssImport::persistSkeleton()
{
	if (_skeleton.get())
	{
		return persistBinary(*_skeleton, { std::string{ _destPath + _sceneName + "_skeleton" + ".aeon" }, {}, EAssetType::SKELETON }, *_pLedger);
	}
	return NULL_ASSET;
}


std::vector<AssetID> AssImport::persistAnims()
{
	std::vector<AssetID> animIDs;
	for (UINT i = 0; i < _anims.size(); ++i)
	{
		const std::string animName = (_anims[i].getName().size() == 0 ? std::to_string(i) : _anims[i].getName()) + "_anim";
		const std::string animPath{ _destPath + animName + ".aeon" };

		AssetID animID = persistBinary(_anims[i], { animPath, {}, EAssetType::ANIMATION }, *_pLedger);
		animIDs.push_back(animID);
	}
	return animIDs;
}


std::map<std::string, AssetID> AssImport::persistUniqueTextures()
{
	std::map<std::string, AssetID> textureIDs;

	for (auto& namedTextureData : _matData._textures)
	{
		auto& [name, pair] = namedTextureData;
		auto& [tex, rawTexData] = pair;

		std::string texName = std::filesystem::path(name).filename().string();
		std::string texPath{ _destPath + texName };

		textureIDs.insert({ name, persistTexture(texPath, rawTexData) });
	}
	return textureIDs;
}


AssetID AssImport::persistTexture(const std::string& path, const MatImporter::RawTextureData& rawTexData)
{
	FileUtils::writeAllBytes(path, rawTexData.blob.data(), rawTexData.blob.size());
	AssetID textureID = _pLedger->insert({ path, {}, EAssetType::TEXTURE });
	return textureID;
}


std::vector<AssetID> AssImport::persistMats()
{
	// Materials depend on textures so it makes sense to persist them here
	auto textureIDs = persistUniqueTextures();

	std::vector<AssetID> matIDs;

	for (auto i = 0; i < _matData._materials.size(); ++i)
	{
		// Get texture IDs
		auto& matMetaData = _matData._materialData[i];

		MaterialAsset matAsset;

		// Hardcoded for now, fix
		matAsset._opaque = true;

		matAsset._textures.reserve(matMetaData._tempTexData.size());

		std::vector<AssetID> thisMatsTextureIDs;
		thisMatsTextureIDs.reserve(10);

		std::transform(matMetaData._tempTexData.begin(), matMetaData._tempTexData.end(), std::back_inserter(matAsset._textures),
			[&](MatImporter::TexturePathAndMetadata& texPathAndMetaData)
			{
				AssetID textureID = textureIDs.at(texPathAndMetaData.path);
				thisMatsTextureIDs.push_back(textureID);
				return MaterialAsset::TextureRef{ texPathAndMetaData.metaData, textureID };
			});


		matAsset._shaderIDs = _shadersPerMaterial[i];

		// Serialize
		std::string matPath{ _destPath + _sceneName + "_mat_" + std::to_string(i) + ".aeon" };
		AssetID matID = persistBinary<MaterialAsset, cereal::JSONOutputArchive>(matAsset, { matPath, thisMatsTextureIDs, EAssetType::MATERIAL }, *_pLedger);

		matIDs.push_back(matID);
	}
	return matIDs;
}


std::unique_ptr<ModelAsset> AssImport::makeModelAsset(Model& model, std::vector<AssetID> matIDs)
{
	auto modelAsset = std::make_unique<ModelAsset>();
	modelAsset->transform = SMatrix{};	// Review this, whether the transform should even exist and what is it

	for (auto i = 0; i < model._meshes.size(); ++i)
	{
		auto& mesh = model._meshes[i];
		modelAsset->meshes.push_back(MeshAsset{ mesh._vertSig, mesh._vertices, mesh._indices, matIDs[_modelData.meshMaterialMapping[i]] });
	}

	modelAsset->meshNodes = model._meshNodeTree;

	return modelAsset;
}


std::unique_ptr<SkModelAsset> AssImport::makeSkModelAsset(SkModel& skModel, std::vector<AssetID> matIDs, std::vector<AssetID> animIDs, AssetID skeletonID)
{
	auto skModelAsset = std::make_unique<SkModelAsset>();
	auto& modelAsset = skModelAsset->model;

	modelAsset.transform = SMatrix{};

	for (auto i = 0; i < skModel._meshes.size(); ++i)
	{
		auto& mesh = skModel._meshes[i];
		modelAsset.meshes.push_back(MeshAsset{ mesh._vertSig, mesh._vertices, mesh._indices, matIDs[_skModelData.meshMaterialMapping[i]] });
	}

	modelAsset.meshNodes = skModel._meshNodeTree;

	skModelAsset->animations = animIDs;
	skModelAsset->skeleton = skeletonID;

	return skModelAsset;
}


void AssImport::draw(ID3D11DeviceContext* context, float dTime)
{
	auto [columns, rows, spacing] = _numToDraw;
	float numDrawn = columns * rows;
	float fakeDTime = dTime / numDrawn;

	// Horribly inefficient but cba improving it here, make renderer work well and just plug the data in
	for (int i = 0; i < numDrawn; ++i)
	{
		SVec3 offset = SVec3(static_cast<float>(i % columns), 0, static_cast<float>(i / columns)) * spacing;

		if (_skModelData)
		{
			_skModelInst->update(fakeDTime * _playbackSpeed, _currentAnim);
			
			Math::SetTranslation(_skModelInst->_transform, offset);
			Math::SetScale(_skModelInst->_transform, SVec3(_previewScale));

			_skModelInst->draw(context);
		}

		if (_modelData)
		{
			auto& model = _modelData.model;

			Math::SetTranslation(model->_transform, offset);
			Math::SetScale(model->_transform, SVec3(_previewScale));

			for (auto& meshNode : model->_meshNodeTree)
			{
				meshNode.transform = model->_transform * meshNode.transform;
				SMatrix meshNodeTf = meshNode.transform.Transpose();
				for (auto meshIdx : meshNode.meshes)
				{
					auto& mesh = model->_meshes[meshIdx];
					mesh._material->getVS()->updateCBufferDirectly(context, &meshNodeTf, 0);
					model->_meshes[meshIdx].draw(context);
				}
			}
		}
	}
}