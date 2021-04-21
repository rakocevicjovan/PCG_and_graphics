#pragma once
#include "TextureRole.h"

#include <cereal/cereal.hpp>
//#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>

// Used to describe how a texture is used within the context of a material - sampling, binding, role etc.
struct TextureMetaData
{
	TextureRole _role{ DIFFUSE };
	std::array<TextureMapMode, 3> _mapMode{ TextureMapMode::WRAP, TextureMapMode::WRAP, TextureMapMode::WRAP };
	uint8_t _uvIndex{ 0u };
	uint8_t _regIndex{ 0u };	// Decouple from role later


	template <typename Archive>
	void save(Archive& ar) const
	{
		ar(_role, _mapMode, _uvIndex, _regIndex);
	}

	template <typename Archive>
	void load(Archive& ar)
	{
		ar(_role, _mapMode, _uvIndex, _regIndex);
	}
};