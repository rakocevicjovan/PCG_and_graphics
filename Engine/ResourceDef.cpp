#pragma once
#include "ResourceDef.h"



ResourceDef ResourceDef::Load(rapidjson::Value::ConstValueIterator itr)
{
	ResourceDef rd;
	rd._ID = itr->FindMember("id")->value.GetInt();
	rd._path = itr->FindMember("path")->value.GetString();
	rd._assetName = itr->FindMember("name")->value.GetString();
	rd._resType = getResTypeFromString(itr->FindMember("resType")->value.GetString());
	return rd;
}



ResType ResourceDef::getResTypeFromString(const std::string& str)
{
	auto it = resTypeMap.find(str);

	if (it == resTypeMap.end())
		return ResType::UNSUPPORTED;

	return it->second;
}



const std::map<std::string, ResType> ResourceDef::resTypeMap =
{
	{"model",		ResType::MODEL},
	{"texture",		ResType::TEXTURE},
	{"material",	ResType::MATERIAL},
	{"skeleton",	ResType::SKELETON},
	{"animation",	ResType::ANIMATION},
	{"sound",		ResType::SOUND}
};