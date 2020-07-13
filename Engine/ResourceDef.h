#pragma once
#include <rapidjson/document.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <string>
#include <map>
#include "Fnv1Hash.h"


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
	struct RDKey
	{
		uint32_t _ID;	// Consider a bigger ID, or possibly make resType a part of the ID
		std::string _assetName;
	} key;
	
	struct RDVal
	{
		std::string _path;
		ResType _resType;
	} val;
	

	static ResourceDef Load(rapidjson::Value::ConstValueIterator itr);
	
	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(key._ID, key._assetName, val._path, val._resType);
	}

	inline bool operator==(const ResourceDef& other) const
	{
		return (key._ID == other.key._ID);
	}

	static ResType getResTypeFromString(const std::string& str);
	static const std::map<std::string, ResType> RES_TYPE_MAP;
};


template<> struct std::hash<ResourceDef>
{
	std::size_t operator()(const ResourceDef& rd) const noexcept
	{
		//return std::hash<std::string>()(rd.key._assetName);
		return rd.key._ID; // I already use a hashed value so just use that.
	}
};