#pragma once
#include <map>
#include <string>

enum class ResType : uint8_t
{
	MESH = 0u,
	MODEL,
	SK_MESH,
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

inline static const std::map<std::string, ResType> RES_TYPE_MAP
{
	{ "model",		ResType::MODEL		},
	{ "texture",	ResType::TEXTURE	},
	{ "material",	ResType::MATERIAL	},
	{ "skeleton",	ResType::SKELETON	},
	{ "animation",	ResType::ANIMATION	},
	{ "sound",		ResType::SOUND		}
};

inline static ResType ResTypeFromString(const std::string& str)
{
	auto it = RES_TYPE_MAP.find(str);

	if (it == RES_TYPE_MAP.end())
		return ResType::UNSUPPORTED;

	return it->second;
}