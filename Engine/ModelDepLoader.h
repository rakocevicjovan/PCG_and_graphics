#pragma once
#include "ModelLoader.h"
#include "TextureLoader.h"
#include "MaterialLoader.h"
#include "ShaderManager.h"


class ModelDepLoader
{
public:
	
	AssetLedger* assetLedger;
	ShaderManager* shaderManager;


	std::unique_ptr<SkModel> loadSkModel(AssetID assetID)
	{
		auto filePath = assetLedger->get(assetID);

		auto skModelAsset = AssetHelpers::DeserializeFromFile<SkModelAsset>(filePath->c_str());

		auto skModel = ModelLoader::LoadSkModelFromAsset(skModelAsset, *assetLedger);

		for (Mesh& skMesh : skModel->_meshes)
		{
			Material* skMat = skMesh.getMaterial();
			auto shPack = shaderManager->getShaderByData(skMesh._vertSig, skMat);
			skMat->setVS(shPack->vs);
			skMat->setPS(shPack->ps);
		}
	}


	std::unique_ptr<Material> loadMaterial(AssetID assetID)
	{

	}

};