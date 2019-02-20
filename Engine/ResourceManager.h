#pragma once
#include <vector>
#include <map>
#include <string>

#include "Texture.h"
#include "Model.h"



class ResourceManager
{

	class Level {
		std::map<std::string, Mesh> mishmash;
		std::vector<std::pair<unsigned int, Texture>> textures;
	};

public:
	ResourceManager();
	~ResourceManager();

	std::vector<std::pair<std::string,Level>> levels;


	void loadTexture(std::string path) 
	{

	}

};

