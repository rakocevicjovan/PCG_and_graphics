#pragma once
#include "ShaderCompiler.h"
#include "FileUtilities.h"
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
		!!! GUESS WHO GOT CLUSTERED SHADING, ABOVE IS SOLVED
		- Shadow maps - how many, if any
		- Diffuse - color or texture
		- Specular - specular power, specular texture, shininess texture
		- Distance fog - yes or no
		- Gamma corrected - yes or no

	OutputType:
		- data type per channel, number of channels
*/



class ShaderGenerator
{

	struct OptionSet
	{
		std::vector<D3D_SHADER_MACRO> _macros;
		std::vector<uint64_t> _bitmasks;
	};

public:

	ShaderGenerator(){}

	static OptionSet getVsOptions()
	{
		// As far as I can tell from testing, what you pass to it does not matter one bit
		// hlsl preprocessor can only check whether something is defined or not, and not the value...
		//D3D_SHADER_MACRO example = { "name", "definition" };

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



	static void createShPerm(ID3DBlob* textBuffer, const std::vector<D3D_SHADER_MACRO>& permOptions, uint64_t total)
	{
		HRESULT res;
		ID3DBlob* preprocessedBuffer = nullptr;
		ID3DBlob* errorMessage = nullptr;


		res = D3DPreprocess(textBuffer->GetBufferPointer(), textBuffer->GetBufferSize(),
			nullptr, permOptions.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
			&preprocessedBuffer, &errorMessage);

		std::string finalFileName = "ShGen\\GeneratedVS\\vs_" + std::to_string(total) + ".hlsl";

		FileUtils::writeAllBytes(finalFileName.c_str(),
			preprocessedBuffer->GetBufferPointer(),
			preprocessedBuffer->GetBufferSize());

		if (preprocessedBuffer)
		{
			preprocessedBuffer->Release();
			preprocessedBuffer = nullptr;
		}

		if (errorMessage)
		{
			errorMessage->Release();
			errorMessage = nullptr;
		}
	}


	// L"ShGen\\genVS.hlsl" and OptionSet optionSet = getVsOptions(); are defaults
	static bool preprocessAllPermutations(const std::wstring& filePathW, OptionSet optionSet)	
	{
		std::set<uint64_t> _existing;
		std::string filePath = "ShGen\\genVS.hlsl";

		ID3DBlob* textBuffer = nullptr;
		
		if (FAILED(D3DReadFileToBlob(filePathW.c_str(), &textBuffer)))
		{
			OutputDebugStringA("Couldn't read shader template file.");
			exit(2001);
		}

		UINT optionCount = optionSet._macros.size();

		std::vector<D3D_SHADER_MACRO> matchingPermOptions;
		matchingPermOptions.reserve(optionCount);

		for (uint64_t i = 0; i < ( 1 << (optionCount) ); ++i)	//0 - 255, or rather 00000000 to 11111111 loop
		{
			uint64_t total = 0;
			std::string permOptDebugString;

			// Iterate through all options and add macros of those that are in this permutation to
			// the list of the macros passed to the shader compiler
			for (int j = 0; j < optionCount; ++j)
			{
				uint64_t currentOptionBitmask = optionSet._bitmasks[j];
				uint64_t andResult = currentOptionBitmask & i;
				total += andResult;

				// If current option fits the bitmask, add it in
				if (andResult == currentOptionBitmask)
				{
					matchingPermOptions.push_back(optionSet._macros[j]);
					permOptDebugString += optionSet._macros[j].Name;
					permOptDebugString += " ";
				}
			}

			// I barely remember how this works but it should eliminate doubles?
			if (_existing.count(total))
			{
				matchingPermOptions.clear();
				continue;
			}
			_existing.insert(total);

			matchingPermOptions.push_back({ NULL, NULL });	// Required by d3d api

			createShPerm(textBuffer, matchingPermOptions, total);

			matchingPermOptions.clear();

			permOptDebugString += "\n";
			OutputDebugStringA(permOptDebugString.c_str());	
		}

		if (textBuffer)
		{
			textBuffer->Release();
			textBuffer = nullptr;
		}

		return true;
	}
};


// Although these can be separate input options, there is no need for that. 
// If the shader requires tex coordinate output, it must have them (generally speaking) as input anyways...
// therefore we save on the number of permutations - same for normals and tangents
//D3D_SHADER_MACRO i_tex = { "TEX", "true" };	
//D3D_SHADER_MACRO i_nrm = { "NRM", "true" };
//D3D_SHADER_MACRO i_tan = { "TAN", "true" };