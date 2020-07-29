#pragma once
#include <vector>

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


static const std::vector<const char*> TEX_ROLE_NAMES
{ 
	{ "Diffuse"		}, 
	{ "NORMAL"		},
	{ "Specular"	},
	{ "Shininess"	},
	{ "Opacity"		},
	{ "Displacement"},
	{ "Ambient"		},
	{ "Metallic"	},
	{ "Roughness"	},
	{ "Other"		}
};