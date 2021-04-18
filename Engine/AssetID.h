#pragma once
#include "ResourceType.h"

using AssetID = uint32_t;

struct ResourceDef
{
	AssetID _id;
	std::string _path;
	ResType _type;

	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(_id, _path, _type);
	}
};