#pragma once
#include "ResourceHandle.h"	// TBD, should act as a proxy as well?
#include "ResourceDef.h"
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_set.hpp>
#include <unordered_set>
#include <string>
#include <fstream>


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

	using AssetID = uint32_t;

	std::string _ledgerFilePath;

	AssetLedger() : _dirty(false) {}


	~AssetLedger()
	{
		if(_dirty)
			save();	// Is this smart? Hopefully
	}



	AssetID insert(const std::string& assName, const std::string& path, ResType resType)
	{
		AssetID nameHash = fnv1hash(path.c_str());
		ResourceDef rd{ nameHash, assName, path, resType };

		auto iter = _assDefs.insert(rd);
		if (!iter.second)
		{
			// Too heavy handed, but needs a visible warning...
			assert(false && "HASH COLLISION! Asset name: %s", assName);
			//OutputDebugStringA(std::string("HASH COLLISION!" + assName).c_str());
			//return iter.first->key._ID;
		}
		else
		{
			_dirty = true;
			return nameHash;
		}
	}



	inline const ResourceDef* get(const std::string& assPath) const
	{
		return get(fnv1hash(assPath.c_str()));
	}



	inline const ResourceDef* get(AssetID ID) const
	{
		auto iter = _assDefs.find(ResourceDef{ ID });
		
		if (iter != _assDefs.end())
			return &(*iter);
		else
			return nullptr;
	}



	inline void remove(const std::string& assName)
	{
		remove(fnv1hash(assName.c_str()));
	}



	inline void remove(AssetID ID)
	{
		if(_assDefs.erase(ResourceDef{ID, 0}))
			_dirty = true;
	}



	void save()
	{
		std::ofstream ofs(_ledgerFilePath);
		cereal::JSONOutputArchive joArch(ofs);
		serialize(joArch);
	}



	void load()
	{
		std::ifstream ifs(_ledgerFilePath);
		cereal::JSONInputArchive jiArch(ifs);
		serialize(jiArch);
	}



	void purge()
	{
		_assDefs.clear();
		save();
	}
};