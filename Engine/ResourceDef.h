#pragma once
#include <rapidjson/document.h>
#include <string>
#include <map>


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
	UNSUPPORTED
};

struct ResourceDef
{
	uint32_t _ID;	// Consider a higher ID, possibly make resType a part of ID
	std::string _path;
	std::string _assetName;
	ResType _resType;

	static ResourceDef Load(rapidjson::Value::ConstValueIterator itr);
	static ResType getResTypeFromString(const std::string& str);
	static const std::map<std::string, ResType> resTypeMap;
};