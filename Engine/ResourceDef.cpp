#include "pch.h"
#include "ResourceDef.h"
#include <unordered_set>



ResType ResourceDef::getResTypeFromString(const std::string& str)
{
	auto it = RES_TYPE_MAP.find(str);

	if (it == RES_TYPE_MAP.end())
		return ResType::UNSUPPORTED;

	return it->second;
}



const std::map<std::string, ResType> ResourceDef::RES_TYPE_MAP =
{
	{"model",		ResType::MODEL		},
	{"texture",		ResType::TEXTURE	},
	{"material",	ResType::MATERIAL	},
	{"skeleton",	ResType::SKELETON	},
	{"animation",	ResType::ANIMATION	},
	{"sound",		ResType::SOUND		}
};