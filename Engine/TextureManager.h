#pragma once

#include "TCachedLoader.h"
#include "Texture.h"


class TextureManager final : public TCachedLoader<Texture, TextureManager>
{
private:

	ID3D11Device* _device{ nullptr };


public:

	using base = TCachedLoader<Texture, TextureManager>;
	using TCachedLoader<Texture, TextureManager>::TCachedLoader;


	TextureManager(AssetLedger& ledger, AeonLoader& aeonLoader, ID3D11Device* device)
		: base::TCachedLoader(ledger, aeonLoader), _device(device)
	{}


	Texture loadImpl(const char* path)
	{
		return Texture(_device, path);
	}
};