#pragma once

enum class EAssetType : uint8_t
{
	MODEL = 0u,
	SK_MODEL,
	ANIMATION,
	SKELETON,
	MATERIAL,
	SHADER,
	TEXTURE,
	SOUND,
	TEXT,
	SCRIPT,
	UNSUPPORTED,
	UNKNOWN
};

inline const std::map<const char*, EAssetType> ASSET_TYPE_MAP
{
	{ "model",		EAssetType::MODEL		},
	{ "texture",	EAssetType::TEXTURE	},
	{ "material",	EAssetType::MATERIAL	},
	{ "skeleton",	EAssetType::SKELETON	},
	{ "animation",	EAssetType::ANIMATION	},
	{ "sound",		EAssetType::SOUND		}
};

inline EAssetType ResTypeFromString(const char* str)
{
	auto it = ASSET_TYPE_MAP.find(str);

	if (it == ASSET_TYPE_MAP.end())
	{
		return EAssetType::UNSUPPORTED;
	}

	return it->second;
}

template<typename AssetClass>
inline constexpr EAssetType AssetTypeToEnum() = delete;

template<> inline constexpr EAssetType AssetTypeToEnum<class Model>(){return EAssetType::MODEL;}
template<> inline constexpr EAssetType AssetTypeToEnum<class SkModel>() { return EAssetType::SK_MODEL; }
template<> inline constexpr EAssetType AssetTypeToEnum<class Animation>() { return EAssetType::ANIMATION; }
template<> inline constexpr EAssetType AssetTypeToEnum<class Skeleton>() { return EAssetType::SKELETON; }
template<> inline constexpr EAssetType AssetTypeToEnum<class Material>() { return EAssetType::MATERIAL; }
template<> inline constexpr EAssetType AssetTypeToEnum<class Shader>() { return EAssetType::SHADER; }
template<> inline constexpr EAssetType AssetTypeToEnum<class Texture>() { return EAssetType::TEXTURE; }

// Types not in the engine yet
//template<> inline constexpr EAssetType AssetTypeToEnum<Soumd>() { return EAssetType::SOUND; }
//template<> inline constexpr EAssetType AssetTypeToEnum<Text>() { return EAssetType::TEXT; }
//template<> inline constexpr EAssetType AssetTypeToEnum<Script>() { return EAssetType::SCRIPT; }