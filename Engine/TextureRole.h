#pragma once
#include <map>

//used as indices into the array where the values indicate texture slot in shaders
//last one indicates nr of textures used by the shader
enum TextureRole
{
	DIFFUSE = 0,
	NORMAL = 1,
	SPECULAR = 2,
	SHININESS = 3,
	OPACITY = 4,
	DISPLACEMENT = 5,
	AMBIENT = 6,
	OTHER = 7,
	NUM_ROLES = 8
};	//NUM_ROLES should always be last, it's used as the size of array of texture roles in shader classes


static const std::map<TextureRole, char const*> TEX_ROLE_MAP = { 
	{DIFFUSE, "Diffuse"}, 
	{NORMAL, "NORMAL"},
	{SPECULAR, "Specular"},
	{SHININESS, "Shininess"},
	{OPACITY, "Opacity"},
	{DISPLACEMENT, "Displacement"},
	{AMBIENT, "Ambient"},
	{OTHER, "Other"},
};