#pragma once
#include <rapidjson/document.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
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
	uint32_t _ID;	// Consider a bigger ID, or possibly make resType a part of the ID
	std::string _path;
	std::string _assetName;
	ResType _resType;

	static ResourceDef Load(rapidjson::Value::ConstValueIterator itr);
	
	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_ID, _path, _assetName, _resType);
	}

	inline bool operator==(const ResourceDef& other) const
	{
		return (_assetName == other._assetName && _ID == other._ID);
	}

	static ResType getResTypeFromString(const std::string& str);
	static const std::map<std::string, ResType> resTypeMap;
};


template<> struct std::hash<ResourceDef>
{
	std::size_t operator()(const ResourceDef& rd) const noexcept
	{
		return std::hash<std::string>()(rd._assetName);
	}
};