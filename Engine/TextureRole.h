#pragma once
#include <map>

// Used as indices into the array of values that indicate texture registers in shaders
enum TextureRole : uint8_t
{
	DIFFUSE = 0,
	NORMAL = 1,
	SPECULAR = 2,
	SHININESS = 3,
	OPACITY = 4,
	DISPLACEMENT = 5,
	AMBIENT = 6,
	METALLIC = 7,
	ROUGHNESS = 8,
	OTHER = 9,
	NUM_ROLES = 10
};	//NUM_ROLES is used as the size of array of texture roles in shader classes


static const std::map<TextureRole, const char*> TEX_ROLE_MAP
{ 
	{DIFFUSE,		"Diffuse"}, 
	{NORMAL,		"NORMAL"},
	{SPECULAR,		"Specular"},
	{SHININESS,		"Shininess"},
	{OPACITY,		"Opacity"},
	{DISPLACEMENT,	"Displacement"},
	{AMBIENT,		"Ambient"},
	{METALLIC,		"Metallic"},
	{OTHER,			"Other"}
};