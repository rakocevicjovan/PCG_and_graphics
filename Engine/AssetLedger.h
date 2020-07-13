#pragma once
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_set.hpp>
#include <unordered_set>
#include <string>
#include <fstream>
#include "ResourceHandle.h"	// TBD, should act as a proxy as well?
#include "ResourceDef.h"



class AssetLedger
{
private:
	
	// Gigabrain implementation with 0 effort, 0 cache locality and maximum fragmentation
	std::unordered_set<ResourceDef> _assDefs;

	// This will write it all out at once, something I don't like one bit (might use sqlite)
	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_assDefs);
	}

public:

	std::string _ledgerFilePath;


	uint32_t add(const std::string& assName, const std::string& path, ResType resType)
	{
		uint32_t result = fnv1hash(assName);
		ResourceDef rd{ result, assName, path, resType };

		if (!_assDefs.insert(rd).second)
		{
			// Heavy handed, needs a very visible warning though.
			assert(false && "HASH COLLISION! Asset name: %s", assName);
		}
		return result;
	}



	const ResourceDef* get(const std::string& assName)
	{
		get(fnv1hash(assName));
	}



	const ResourceDef* get(uint32_t ID)
	{
		auto iter = _assDefs.find(ResourceDef{ ID });
		
		if (iter != _assDefs.end())
		{
			return &(*iter);
		}
		else
		{
			return nullptr;
		}
	}



	void remove(const std::string& assName)
	{
		remove(fnv1hash(assName));
	}



	void remove(uint32_t ID)
	{
		_assDefs.erase(ResourceDef{ID, 0});
	}



	void load()
	{
		std::ifstream ifs(_ledgerFilePath);
		cereal::JSONInputArchive jiArch(ifs);
		serialize(jiArch);
	}


	void save()
	{
		std::ofstream ofs(_ledgerFilePath);
		cereal::JSONOutputArchive joArch(ofs);
		serialize(joArch);
	}


	void purge()
	{
		_assDefs.clear();
		save();
	}
};