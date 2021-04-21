#include "pch.h"
#include "MaterialAsset.h"
//#include "ShaderManager.h"
//#include "TextureManager.h"

Material MaterialAsset::createRuntimeMaterial(ShaderManager* shMan, TextureManager* texMan)
{
	auto* shPack = shMan->getShaderByKey(_shaderKey);

	Material result(shPack->vs, shPack->ps, _opaque);

	for (auto i = 0; i < _textureIDs.size(); ++i)
	{
		Texture* t = texMan->get(_textureIDs[i]);
		result.addMaterialTexture(t, _texMetaData[i]);
	}

	return result;
}