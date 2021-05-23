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


	inline bool contains(AssetID id)
	{
		return _assDefs.find(id) != _assDefs.end();
	}


	inline bool contains(const char* path)
	{
		return contains(fnv1hash(path));
	}


	AssetID insert(const char* path, ResType /*resType*/)
	{
		AssetID pathHash = fnv1hash(path);

		auto iter = _assDefs.insert({ pathHash, path});
		if (!iter.second)
		{
			OutputDebugStringA(std::string("HASH COLLISION!" + std::string{ path, strlen(path) }).c_str());
			return NULL_ASSET;
		}

		_dirty = true;
		return pathHash;
	}


	inline const std::string* get(AssetID ID) const
	{
		auto iter = _assDefs.find(ID);
		
		return iter == _assDefs.end() ? nullptr : &(iter->second);
	}


	AssetID getOrInsert(const char* path, ResType resType)
	{
		auto id = getExistingID(path);
		return getExistingID(path) == NULL_ASSET ? insert(path, resType) : id;
	}


	inline const AssetID getExistingID(const char* path)
	{
		AssetID pathHash = fnv1hash(path);

		return contains(pathHash) ? pathHash : NULL_ASSET;
	}


	inline void remove(AssetID ID)
	{
		if (_assDefs.erase(ID))
		{
			_dirty = true;
		}
	}


	inline void remove(const std::string& assName)
	{
		remove(fnv1hash(assName.c_str()));
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