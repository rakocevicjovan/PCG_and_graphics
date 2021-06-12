#pragma once
#include "FileBrowser.h"
#include "ShaderManager.h"

// Structs representing serialized versions of different runtime types, as well as their importers and loaders
#include "Model.h"
#include "SkeletalModel.h"
#include "SkeletalModelInstance.h"

#include "ModelImporter.h"
#include "ModelAsset.h"
#include "ModelLoader.h"

#include "MatImporter.h"
#include "MaterialAsset.h"
#include "MaterialLoader.h"

template <typename AssetType, typename ArchiveType = cereal::BinaryOutputArchive>
AssetID persistBinary(AssetType& asset, const std::string& path, ResType resType, AssetLedger& assetLedger)
{
	std::ofstream outputFileStream(path, std::ios::binary);
	ArchiveType binaryOutputArchive(outputFileStream);
	asset.serialize(binaryOutputArchive);
	AssetID result = assetLedger.insert(path.c_str(), resType);
	assetLedger.save();
	return result;
}


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
	ModelImporter::ModelImportData<SkModel> _skModelData;
	ModelImporter::ModelImportData<Model> _modelData;
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

	bool loadAiScene(ID3D11Device* device, const char* importFrom, const char* importTo, AssetLedger& assetLedger, ShaderManager* shMan);

	void importSelectedAssets();

	void displayImportSettings();
	bool displayPreview();
	void displayAiScene();
	void displayParsedAssets();
	bool displayCommands();

	// Eeeeehhhh... weird way to do it.
	void persistAssets();
	std::vector<AssetID> persistMats();
	std::vector<uint32_t> persistAnims();
	AssetID persistSkeleton();
	AssetID persistTexture(const std::string& name, const std::pair<std::shared_ptr<Texture>, MatImporter::RawTextureData>& texture);

	std::unique_ptr<ModelAsset> makeModelAsset(Model& model, std::vector<AssetID> matIDs);
	std::unique_ptr<SkModelAsset> makeSkModelAsset(SkModel& skModel, std::vector<AssetID> matIDs, std::vector<AssetID> animIDs, AssetID skeletonID);

	void draw(ID3D11DeviceContext* context, float dTime);

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