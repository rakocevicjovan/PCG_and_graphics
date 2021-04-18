#pragma once
#include "AssetID.h"
#include "Fnv1Hash.h"
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <unordered_map>
#include <string>
#include <fstream>
#include <cassert>


class AssetLedger
{
private:
	
	bool _dirty{false};

	std::unordered_map<AssetID, std::string> _assDefs;

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


	AssetID insert(const std::string& path, ResType /*resType*/)
	{
		AssetID nameHash = fnv1hash(path.c_str());

		auto iter = _assDefs.insert({nameHash, path});
		if (!iter.second)
		{
			// Too heavy handed, but needs a visible warning...
			assert(false && "HASH COLLISION! Asset path: %s", path);
			//OutputDebugStringA(std::string("HASH COLLISION!" + assName).c_str());
			//return iter.first->key._ID;
		}
		else
		{
			_dirty = true;
			return nameHash;
		}
	}


	inline const std::string* get(AssetID ID) const
	{
		auto iter = _assDefs.find(ID);
		
		if (iter != _assDefs.end())
			return &(iter->second);
		else
			return nullptr;
	}


	inline void remove(const std::string& assName)
	{
		remove(fnv1hash(assName.c_str()));
	}


	inline void remove(AssetID ID)
	{
		if (_assDefs.erase(ID))
		{
			_dirty = true;
		}
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