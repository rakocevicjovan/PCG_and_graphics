#pragma once
#include <vector>

enum TextureMapMode : uint8_t
{
	WRAP,
	MIRROR,
	CLAMP,
	BORDER,
	MIRROR_ONCE	// Available in DX11 but assimp doesn't support it
};


// Used as indices into the array of values that indicate texture registers in shaders
enum TextureRole : uint8_t
{
	DIFFUSE		= 0,
	NORMAL		= 1,
	SPECULAR	= 2,
	SHININESS	= 3,
	OPACITY		= 4,
	DPCM		= 5,
	AMB_OCCLUSION		= 6,
	METALLIC	= 7,
	ROUGHNESS	= 8,
	REFLECTION	= 9,
	REFRACTION	= 10,
	OTHER		= 11,
	NUM_ROLES	= 12
};	//NUM_ROLES is used as the size of array of texture roles in shader classes


// MUST match up with order and number of TextureRole enum
static const std::vector<const char*> TEX_ROLE_NAMES
{ 
	"Diffuse", 
	"Normal",
	"Specular",
	"Shininess",
	"Opacity",
	"Displacement",
	"Ambient",
	"Metallic",
	"Roughness",
	"Reflection",
	"Refraction",
	"Other"
};


static const std::vector<const char*> TEX_MAPMODE_NAMES
{
	"Wrap",
	"Mirror",
	"Clamp",
	"Border",
	"MirrorOnce"
};