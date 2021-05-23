#include "TextureMetaData.h"

struct MaterialAsset
{
	struct AssetMaterialTexture
	{
		TextureMetaData _texMetaData;
		AssetID _textureAssetID;

		template <typename Archive>
		void serialize(Archive& ar)
		{
			ar(_texMetaData, _textureAssetID);
		}
	};

	std::array<AssetID, 6> _shaderIDs{ NULL_ASSET };	// Vertex, pixel, geometry, hull, domain, tasselation

	std::vector<AssetMaterialTexture> _textures;

	bool _opaque{ false };

	MaterialAsset() = default;

	template <typename Archive>
	void serialize(Archive& ar) const
	{
		ar(_shaderIDs, _textures, _opaque);
	}
};