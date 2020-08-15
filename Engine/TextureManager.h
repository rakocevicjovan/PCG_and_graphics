#pragma once
#include "Texture.h"
#include <unordered_map>
#include <map>	// Map allows for proper autocomplete in the editor! unordered map not quite

class TextureManager
{
private:

	typedef std::map<std::string, Texture>::iterator texMapIter;

	std::map<std::string, Texture> _textures;

public:

	void addTexture()
	{

	}


	void getTexture(const std::string& name)
	{

	}


	std::pair<texMapIter, texMapIter> autocomplete(const std::string& name)
	{
		// Probably add check against limits etc.
		auto iter = _textures.equal_range(name);
		return iter;
	}

};