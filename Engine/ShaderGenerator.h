#pragma once
#include "ShaderCompiler.h"

/*
//Vertex shader options:
	
	Basically... these are not separate options, and they can be co dependent - no input/processing for x means no output x either etc.

	Input Layout:
		PER VERTEX
		- pos - always
		- tex coords - consider suporting several sets although I didn't need it so far
		- normals
		- tangents
		PER INSTANCE
		- matrix (might have to rethink this but lets keep it simple for now)
		- animation matrix array

	Process:
		- SV_POSITION - always
		- world pos		//required quite often
		- tex coords	//not required for shadow pass
		- normals		//not required for GUI
		- tangents		//used only for normal mapping in PS?
		- skinning
	Output Layout:
		- SV_POSITION 
		- world pos
		- tex coords
		- normals
		- tangents

// Skip geometry unless it becomes relevant... I use it so rarely that it can be done by hand rather than complicating the generator

// Pixel shader options (oooh boy...)

	Input Layout:
		- same as VS output, unless there is a GS thrown in...

	Process:
		- lighting method (Lambert, Phong, Blinn-Phong, Cook-Torrance, none...) - 4 bits should be ok... I guess?
		- lights - number, types... (this could get out of hand even with an 4/8 light max...)
		- Shadow maps - how many, if any
		- Diffuse - color or texture
		- Specular - specular power, specular texture, shininess texture
		- Distance fog - yes or no
		- Gamma corrected - yes or no

	OutputType:
		- data type per channel, number of channels
*/


struct ShaderOptions
{
	UINT v_in_tex : 1;
	UINT v_in_nrm : 1;
	UINT v_in_tan : 1;
	UINT v_in_ins : 1;

	UINT v_po_wps : 1;
	UINT v_po_tex : 1;
	UINT v_po_nrm : 1;
	UINT v_po_tan : 1;

	//v_PO implies the same p_in, no need for more bits than that

	UINT p_p_lightModel : 4;	//16 possible lighting models, should be plenty
	UINT p_p_nrLights	: 3;	//up to 8 lights, if light model is none it means none will be used
	UINT p_p_lightTypes : 3;	//directional, point, spot, area, volume, ambient... and what?

	UINT p_p_ambient	: 1;

	UINT p_p_textures	: 24;	//don't even know... but it ought to be a lot, got plenty of types and whatnot
	UINT p_p_distFog	: 1;
	UINT p_p_gCorrected : 1;
};


//will be used to create permutations based on nr of lights, inputs etc... additive approach planned
class ShaderGenerator
{
	ShaderCompiler _shc;


	void createDefines()
	{
		D3D_SHADER_MACRO exampleMacro = { "name", "definition" };
	}


};