#pragma once
#include <map>
#include <string>

enum class AssetType : uint8_t
{
	MODEL = 0u,
	SK_MODEL,
	ANIMATION,
	SKELETON,
	MATERIAL,
	TEXTURE,
	SOUND,
	TEXT,
	SCRIPT,
	UNSUPPORTED
};

inline static const std::map<std::string, AssetType> RES_TYPE_MAP
{
	{ "model",		AssetType::MODEL		},
	{ "texture",	AssetType::TEXTURE	},
	{ "material",	AssetType::MATERIAL	},
	{ "skeleton",	AssetType::SKELETON	},
	{ "animation",	AssetType::ANIMATION	},
	{ "sound",		AssetType::SOUND		}
};

inline static AssetType ResTypeFromString(const std::string& str)
{
	auto it = RES_TYPE_MAP.find(str);

	if (it == RES_TYPE_MAP.end())
		return AssetType::UNSUPPORTED;

	return it->second;
}