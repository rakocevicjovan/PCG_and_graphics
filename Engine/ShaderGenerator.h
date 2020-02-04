#pragma once
#include "ShaderCompiler.h"
#include <algorithm>
#include <set>

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


//will be used to create permutations based on nr of lights, inputs etc... additive approach planned
class ShaderGenerator
{

	//kinda blows to initialize tbh... although it is better for performance but this is an offline tool who cares
	struct OptionSet
	{
		std::vector<D3D_SHADER_MACRO> _macros;
		std::vector<uint64_t> _bitmasks;
	};

	ShaderCompiler _shc;
	std::set<uint64_t> _existing;

public:
	ShaderGenerator(ShaderCompiler& shc) : _shc(shc) {}

	OptionSet getAllOptions()
	{
		// As far as I can tell from testing, what you pass to it does not matter one bit
		// hlsl preprocessor can only check whether something is defined or not, and not the value...
		//D3D_SHADER_MACRO example = { "name", "definition" };

		// Although these can be separate, there is no need for that. 
		// If the shader requires tex coordinate output, it must have them (generally speaking) as input anyways...
		// therefore we save on the number of permutations
		//D3D_SHADER_MACRO i_tex = { "TEX", "true" };	
		//D3D_SHADER_MACRO i_nrm = { "NRM", "true" };
		//D3D_SHADER_MACRO i_tan = { "TAN", "true" };

		// Assumes that instance data is a matrix, not very flexible
		D3D_SHADER_MACRO i_ins = { "INS", "true" };

		D3D_SHADER_MACRO o_wps = { "WPS", "true" };	
		D3D_SHADER_MACRO o_tex = { "TEX", "true" };
		D3D_SHADER_MACRO o_nrm = { "NRM", "true" };
		D3D_SHADER_MACRO o_tan = { "TAN", "true" };

		uint64_t instanced	= 1 << 0;
		uint64_t texCoords	= 1 << 1;
		uint64_t normals	= 1 << 2;
		uint64_t worldPos	= 1 << 3;
		uint64_t tangents	= 1 << 4 | normals;

		std::vector<D3D_SHADER_MACRO> macros =	{ i_ins,	 o_tex,		o_nrm,		o_wps,		o_tan };

		std::vector<uint64_t> bitmasks =		{ instanced, texCoords, normals,	worldPos,	tangents };

		//11111 is the current bit mask
		OptionSet result = { macros, bitmasks };

		return result;
	}


	// @TODO handle duplications by keeping a set of inserted bit masks
	// simply store variable "total" in there and check if exists before creating
	// currently this sorta takes care of itself because files rewrite but it is doing unnecessary work
	bool mix()
	{
		std::string filePath = "ShGen\\genVS.hlsl";
		std::wstring filePathW = L"ShGen\\genVS.hlsl";	//microsoft pls...

		ID3DBlob* textBuffer = nullptr;
		ID3DBlob* preprocessedBuffer = nullptr;
		ID3DBlob* errorMessage = nullptr;

		ID3D11VertexShader* vertexShader;
		ID3D11InputLayout* layout;

		OptionSet optionSet = getAllOptions();
		UINT optionCount = optionSet._macros.size();

		std::vector<D3D_SHADER_MACRO> permOptions;
		permOptions.reserve(optionCount);

		for (uint64_t i = 0; i < (1 << (optionCount) ); ++i)	//0 - 255, or rather 00000000 to 11111111 loop
		{
			uint64_t total = 0u;
			std::string permOptDebugString;

			for (int j = 0; j < optionCount; ++j)
			{
				uint64_t andResult = optionSet._bitmasks[j] & i;
				total += andResult;
				if (andResult == optionSet._bitmasks[j])
				{
					permOptions.push_back(optionSet._macros[j]);
					permOptDebugString += optionSet._macros[j].Name;
					permOptDebugString += " ";
				}
			}

			if (_existing.count(total))
			{
				permOptions.clear();
				continue;	//skips unnecessary work, as this shader was already created
			}
			_existing.insert(total);

			permOptions.push_back({ NULL, NULL });

			HRESULT res;

			res = D3DReadFileToBlob(filePathW.c_str(), &textBuffer);
			res = D3DPreprocess(textBuffer->GetBufferPointer(), textBuffer->GetBufferSize(), filePath.c_str(),
				permOptions.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, &preprocessedBuffer, &errorMessage);

			std::ofstream fout;
			std::wstring finalFileName = L"ShGen\\GeneratedVS\\vs_" + std::to_wstring(total) + L".hlsl";
			
			fout.open(finalFileName);
			for (ULONG j = 0; j < preprocessedBuffer->GetBufferSize(); ++j)
				fout << ((char*)(preprocessedBuffer->GetBufferPointer()))[j];

			fout.close();

			preprocessedBuffer->Release();
			preprocessedBuffer = nullptr;

			permOptDebugString += "\n";
			OutputDebugStringA(permOptDebugString.c_str());
		}

		return true;

		// options.data()
		/*if (FAILED(D3DCompileFromFile(targetPath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &preprocessedBuffer, &errorMessage)))
		{
			//outputError(errorMessage, *_hwnd, *(filePath.c_str()), filePath);
			return false;
		}*/

		/*
		if (FAILED(_shc.getDevice()->CreateVertexShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &vertexShader)))
		{
			//MessageBox(*_hwnd, filePath.c_str(), L"Failed to create vertex shader.", MB_OK);
			return false;
		}
		*/
	}
};


//initial key draft, just to brainstorm on
struct ShaderOptions
{
	// VS keys
	UINT v_in_tex : 1;
	UINT v_in_nrm : 1;
	UINT v_in_tan : 1;
	UINT v_in_ins : 1;

	// v_PO implies the same p_in, no need for more bits than that
	UINT v_po_wps : 1;
	UINT v_po_tex : 1;
	UINT v_po_nrm : 1;
	UINT v_po_tan : 1;

	

	// PS keys
	UINT p_p_lightModel : 4;	//16 possible lighting models, should be plenty
	UINT p_p_nrLights	: 3;	//up to 8 lights, if light model is none it means none will be used
	UINT p_p_lightTypes : 3;	//directional, point, spot, area, volume, ambient... and what?

	UINT p_p_ambient : 1;

	UINT p_p_diffMap	: 1;
	UINT p_p_normalMap	: 1;
	UINT p_p_specMap	: 1;
	UINT p_p_shinyMap	: 1;
	UINT p_p_opacityMap : 1;
	UINT p_p_displMap	: 1;
	UINT p_p_aoMap		: 1;
	UINT p_p_roughMap	: 1;
	UINT p_p_metalMap	: 1;
	UINT p_p_iorMap		: 1;

	UINT p_p_distFog	: 1;
	UINT p_p_gCorrected : 1;
};	//well... thats just over 2 billion permutations... somehow I really don't think I need that many :V