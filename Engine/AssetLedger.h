#pragma once
#include "AssetID.h"
#include "Fnv1Hash.h"

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
		//cereal::JSONOutputArchive joArch(ofs);
		//serialize(joArch);
	}


	void load()
	{
		std::ifstream ifs(_ledgerFilePath);
		//cereal::JSONInputArchive jiArch(ifs);
		//serialize(jiArch);
	}


	void purge()
	{
		_assDefs.clear();
		save();
	}
};