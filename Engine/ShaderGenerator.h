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

	UINT p_p_diffMap	: 1;
	UINT p_p_normalMap	: 1;
	UINT p_p_specMap	: 1;
	UINT p_p_shinyMap	: 1;
	UINT p_p_opacityMap	: 1;
	UINT p_p_displMap	: 1;
	UINT p_p_aoMap		: 1;
	UINT p_p_roughMap	: 1;
	UINT p_p_metalMap	: 1;
	UINT p_p_iorMap		: 1;

	UINT p_p_distFog	: 1;
	UINT p_p_gCorrected : 1;
};

//well... thats just over 2 billion permutations... somehow I really don't think I need that many :V


//will be used to create permutations based on nr of lights, inputs etc... additive approach planned
class ShaderGenerator
{
	ShaderCompiler _shc;

public:
	ShaderGenerator(ShaderCompiler& shc) : _shc(shc) {}

	std::vector<D3D_SHADER_MACRO> createDefines()
	{
		D3D_SHADER_MACRO example = { "name", "definition" };

		D3D_SHADER_MACRO i_tex = { "TEX", "true" };
		D3D_SHADER_MACRO i_nrm = { "NRM", "true" };
		D3D_SHADER_MACRO i_tan = { "TAN", "true" };
		D3D_SHADER_MACRO i_ins = { "INS", "true" };

		D3D_SHADER_MACRO o_tex = { "OTEX", "true" };
		D3D_SHADER_MACRO o_nrm = { "ONRM", "true" };
		D3D_SHADER_MACRO o_wps = { "OWPS", "true" };
		D3D_SHADER_MACRO o_tan = { "OTAN", "true" };

		std::vector<D3D_SHADER_MACRO> result =
		{
			i_tex, i_nrm, 
			o_wps, o_tex, o_nrm
		};



		result.push_back({ NULL, NULL });

		return result;
	}


	bool mix()
	{
		ID3DBlob* precompiled = nullptr;
		ID3DBlob* shaderBuffer = nullptr;
		ID3DBlob* errorMessage = nullptr;


		ID3D11VertexShader* vertexShader;
		ID3D11InputLayout* layout;

		std::vector<D3D_SHADER_MACRO> options = createDefines();
		const D3D_SHADER_MACRO* macroArray = options.data();

		std::string filePath = "ShGen\\genVS.hlsl";
		std::wstring filePathW = L"ShGen\\genVS.hlsl";

		std::wstring targetPath = L"ShGen\\generated.hlsl";


		char* shaderText;
		unsigned long bufferSize, i;
		std::ofstream fout;

		HRESULT res;

		res = D3DReadFileToBlob(filePathW.c_str(), &precompiled);
		res = D3DPreprocess(precompiled->GetBufferPointer(), precompiled->GetBufferSize(), filePath.c_str(), 
			macroArray, D3D_COMPILE_STANDARD_FILE_INCLUDE, &shaderBuffer, &errorMessage);

		shaderText = (char*)(shaderBuffer->GetBufferPointer());
		bufferSize = shaderBuffer->GetBufferSize();

		fout.open(targetPath);
		for (i = 0; i < bufferSize; i++)
			fout << shaderText[i];

		fout.close();

		shaderBuffer->Release();
		shaderBuffer = nullptr;


		// options.data()
		if (FAILED(D3DCompileFromFile(targetPath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &shaderBuffer, &errorMessage)))
		{
			//outputError(errorMessage, *_hwnd, *(filePath.c_str()), filePath);
			return false;
		}

		return true;

		/*
		if (FAILED(_shc.getDevice()->CreateVertexShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &vertexShader)))
		{
			//MessageBox(*_hwnd, filePath.c_str(), L"Failed to create vertex shader.", MB_OK);
			return false;
		}
		*/

		/*
		if (FAILED(_device->CreateInputLayout(inLay.data(), inLay.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &layout)))
		{
			//MessageBox(*_hwnd, filePath.c_str(), L"Failed to create vertex input layout.", MB_OK);
			return false;
		}
		*/
	}


};