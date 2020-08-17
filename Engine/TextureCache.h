#pragma once
#include "Texture.h"
#include <memory>
//#include <unordered_map>// Map allows proper autocomplete in the editor
#include <map>	

class TextureCache
{
private:

	typedef std::map<std::string, std::unique_ptr<Texture>>::iterator TexMapIter;
	typedef std::pair<TexMapIter, TexMapIter> TexMapRange;

	std::map<std::string, std::unique_ptr<Texture>> _textures;

public:


	bool addTexture(const std::string& name, Texture* t)
	{
		auto insertResult = _textures.insert({ name, std::make_unique<Texture>(t) });
		return insertResult.second;
	}



	void getTexture(const std::string& name)
	{

	}



	TexMapRange autocomplete(const std::string& name)
	{
		// Probably add check against limits etc.
		auto iter = _textures.equal_range(name);
		return iter;
	}
};