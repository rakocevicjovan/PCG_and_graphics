#include "TextureMetaData.h"

struct MaterialAsset
{
	std::array<AssetID, 6> _shaderIDs{ NULL_ASSET };	// Vertex, pixel, geometry, hull, domain, tasselation

	std::vector<TextureMetaData> _texMetaData;
	std::vector<AssetID> _textureIDs;	// Include material-related meta data as well!

	bool _opaque{ false };

	MaterialAsset() = default;

	template <typename Archive>
	void serialize(Archive& ar) const
	{
		ar(_shaderIDs, _texMetaData, _textureIDs, _opaque);
	}
};