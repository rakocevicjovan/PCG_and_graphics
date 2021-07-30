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

	void getDepsRecursive(const AssetMetaData* amd, std::vector<const AssetMetaData*>& outDeps) const
	{
		auto& immediateDepIDs = amd->dependencies;

		// If we only wanted asset IDs
		//std::copy(immediateDepIDs.begin(), immediateDepIDs.end(), std::back_inserter(outDeps));

		auto first = outDeps.size();
		auto last = first + immediateDepIDs.size();

		std::transform(immediateDepIDs.begin(), immediateDepIDs.end(), std::back_inserter(outDeps),
			[this](AssetID curDepID)
			{
				return get(curDepID);
			});

		for (auto i = first; i < last; ++i)
		{
			getDepsRecursive(outDeps[i], outDeps);
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


	std::vector<const AssetMetaData*> getAllDependencies(AssetID ID, uint32_t numDepsHint = 10u) const
	{
		std::vector<const AssetMetaData*> result;
		result.reserve(numDepsHint);

		//result.push_back(ID); // Include self for convenience?
		getDepsRecursive(get(ID), result);

		// Questionable if this should be done here
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