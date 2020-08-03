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
	bool _dirty;

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

	AssetLedger() : _dirty(false) {}


	~AssetLedger()
	{
		if(_dirty)
			save();	// Is this smart? Hopefully
	}



	uint32_t add(const std::string& assName, const std::string& path, ResType resType)
	{
		uint32_t result = fnv1hash(assName);
		ResourceDef rd{ result, assName, path, resType };

		if (!_assDefs.insert(rd).second)
		{
			// Heavy handed, needs a very visible warning though.
			assert(false && "HASH COLLISION! Asset name: %s", assName);
		}

		_dirty = true;

		return result;
	}



	inline const ResourceDef* get(const std::string& assName) const
	{
		get(fnv1hash(assName));
	}



	inline const ResourceDef* get(uint32_t ID) const
	{
		auto iter = _assDefs.find(ResourceDef{ ID });
		
		if (iter != _assDefs.end())
			return &(*iter);
		else
			return nullptr;
	}



	inline void remove(const std::string& assName)
	{
		remove(fnv1hash(assName));
	}



	inline void remove(uint32_t ID)
	{
		if(_assDefs.erase(ResourceDef{ID, 0}))
			_dirty = true;
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