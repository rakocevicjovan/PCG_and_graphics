#pragma once
#include "TextureRole.h"

// Used to describe how a texture is used within the context of a material - sampling, binding, role etc.
struct TextureMetaData
{
	TextureRole _role{ DIFFUSE };
	std::array<SamplingMode, 3> _mapMode{ SamplingMode::WRAP };
	uint8_t _uvIndex{ 0u };
	uint8_t _regIndex{ 0u };	// Decouple from role later


	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_role, _mapMode, _uvIndex, _regIndex);
	}
};