#pragma once
#include "AssetID.h"
#include "Fnv1Hash.h"


struct AssetMetaData
{
	std::string path;
	std::vector<AssetID> dependencies;
	AssetType type;

	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(path, dependencies, type);
	}
};


class AssetLedger
{
private:
	
	bool _dirty{false};

	std::unordered_map<AssetID, AssetMetaData> _assDefs;

	// This will write it all out at once, something I don't like one bit (might use sqlite)
	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_assDefs);
	}

	void getDepsRecursive(AssetID ID, std::vector<AssetID>& outDeps) const
	{
		auto& immediateDeps = get(ID)->dependencies;

		std::copy(immediateDeps.begin(), immediateDeps.end(), std::back_inserter(outDeps));

		for (auto& dep : immediateDeps)
		{
			getDepsRecursive(dep, outDeps);
		}
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


	AssetID insert(const AssetMetaData& amd)
	{
		AssetID pathHash = fnv1hash_64(amd.path.c_str());

		auto iter = _assDefs.insert({ pathHash, amd });
		if (!iter.second)
		{
			OutputDebugStringA(std::string("HASH COLLISION!" + amd.path).c_str());
			return NULL_ASSET;
		}

		_dirty = true;
		return pathHash;
	}


	inline const AssetMetaData* get(AssetID ID) const
	{
		auto iter = _assDefs.find(ID);
		
		return iter == _assDefs.end() ? nullptr : &(iter->second);
	}


	inline const std::string* getPath(AssetID ID) const
	{
		return &(get(ID)->path);
	}


	const std::vector<AssetID>&& getAllDependencies(AssetID ID) const
	{
		static std::vector<AssetID> result;
		result.clear();
		result.reserve(10);

		//result.push_back(ID); // Include self for convenience?
		getDepsRecursive(ID, result);

		std::sort(result.begin(), result.end());
		result.erase(std::unique(result.begin(), result.end()), result.end());

		return std::move(result);
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