#pragma once


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
	OTHER = 6,
	NUM_ROLES = 7
};	//NUM_ROLES should always be last, it's used as the size of array of texture roles in shader classes