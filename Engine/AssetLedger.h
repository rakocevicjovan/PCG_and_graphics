#pragma once

#include "AssetID.h"
#include "Fnv1Hash.h"


struct AssetMetaData
{
	std::string path;
	std::vector<AssetID> dependencies;
	EAssetType type{ EAssetType::UNKNOWN };

	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(path, dependencies, type);
	}
};


class AssetLedger
{
public:
	
	bool _dirty{false};

	std::unordered_map<AssetID, AssetMetaData> _assetDefinitions;

	// This will write it all out at once, something I don't like one bit (might use sqlite)
	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_assetDefinitions);
	}

	void getFullDependencyTree(const AssetMetaData* amd, std::vector<const AssetMetaData*>& outDeps) const
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
			getFullDependencyTree(outDeps[i], outDeps);
		}
	}

public:

	std::string _ledgerFilePath{};

	AssetLedger() = default;

	~AssetLedger()
	{
		// Temporary behaviour
		if(_dirty)
		{
			save();	
		}
	}


	inline bool contains(AssetID id)
	{
		return _assetDefinitions.find(id) != _assetDefinitions.end();
	}


	AssetID getOrInsert(const AssetMetaData& amd)
	{
		const auto inputAssetID = CreateAssetID(fnv1hash(amd.path.c_str()), amd.type, true);
		
		const auto [assetEntry, inserted] = _assetDefinitions.try_emplace(inputAssetID, amd);
		
		auto assetID = assetEntry->first;

		_dirty |= inserted;

		return assetID;
	}


	AssetID insert(const AssetMetaData& amd)
	{
		const auto inputAssetID = CreateAssetID(fnv1hash(amd.path.c_str()), amd.type, true);

		auto iter = _assetDefinitions.insert({ inputAssetID, amd });
		if (!iter.second)
		{
			OutputDebugStringA(std::string("HASH COLLISION!" + amd.path).c_str());
			return NULL_ASSET;
		}

		_dirty = true;
		return inputAssetID;
	}


	inline const AssetMetaData* get(AssetID ID) const
	{
		auto iter = _assetDefinitions.find(ID);
		
		return iter == _assetDefinitions.end() ? nullptr : &(iter->second);
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
		getFullDependencyTree(get(ID), result);

		// Questionable if this should be done here
		std::sort(result.begin(), result.end());
		result.erase(std::unique(result.begin(), result.end()), result.end());

		return result;
	}


	inline void remove(AssetID ID)
	{
		if (_assetDefinitions.erase(ID))
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
		_assetDefinitions.clear();
		save();
	}
};