#pragma once
#include "Texture.h"
#include <string>
#include <memory>
#include <map>	

class TextureCache
{
private:

	typedef std::map<std::string, std::unique_ptr<Texture>>::iterator TexMapIter;
	typedef std::pair<TexMapIter, TexMapIter> TexMapRange;

	// Map allows proper autocomplete in the editor, as opposed to unordered_map
	std::map<std::string, std::unique_ptr<Texture>> _textures;

public:

	
	bool addTexture(std::string& name, Texture* tex)
	{
		std::unique_ptr<Texture> upTexture;
		upTexture.reset(tex);
		auto insertResult = _textures.insert(std::make_pair(name, std::move(upTexture)));
		return insertResult.second;
	}


	
	Texture* getTexture(const std::string& name)
	{
		auto iter = _textures.find(name);
		if (iter != _textures.end())
			return iter->second.get();
		return nullptr;
	}



	TexMapRange autocomplete(const std::string& name)
	{
		// Probably add check against limits etc.
		auto iter = _textures.equal_range(name);
		return iter;
	}
	
	
};