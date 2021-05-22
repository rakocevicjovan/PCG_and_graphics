struct MaterialAsset
{
	std::array<AssetID, 6> _shaderIDs{ NULL_ASSET };	// Vertex, pixel, geometry, hull, domain, tasselation
	std::vector<AssetID> _textureIDs;	// Include material-related meta data as well!

	D3D11_PRIMITIVE_TOPOLOGY _primitiveTopology{ D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	bool _opaque{ false };

	template <typename Archive>
	void save(Archive& ar) const
	{

		ar(_shaderIDs, _textureIDs, _primitiveTopology, _opaque, _materialTextures, texIDs);
	}

	template <typename Archive>
	void load(Archive& ar)
	{
		ar(_shaderIDs, _textureIDs, _primitiveTopology, _opaque, _materialTextures, _texIDs);
	}
};