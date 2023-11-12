#pragma once
#include <map>
#include <string>

enum class EAssetType : uint8_t
{
	MODEL = 0u,
	SK_MODEL,
	ANIMATION,
	SKELETON,
	MATERIAL,
	SHADER,
	TEXTURE,
	SOUND,
	TEXT,
	SCRIPT,
	UNSUPPORTED,
	UNKNOWN
};

inline static const std::map<std::string, EAssetType> RES_TYPE_MAP
{
	{ "model",		EAssetType::MODEL		},
	{ "texture",	EAssetType::TEXTURE	},
	{ "material",	EAssetType::MATERIAL	},
	{ "skeleton",	EAssetType::SKELETON	},
	{ "animation",	EAssetType::ANIMATION	},
	{ "sound",		EAssetType::SOUND		}
};

inline static EAssetType ResTypeFromString(const std::string& str)
{
	auto it = RES_TYPE_MAP.find(str);

	if (it == RES_TYPE_MAP.end())
		return EAssetType::UNSUPPORTED;

	return it->second;
}