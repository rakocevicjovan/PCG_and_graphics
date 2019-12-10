#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "ShaderCompiler.h"


enum class SHADER_TYPE { VS, GS, PS};



class Shader
{
	static UINT ID_COUNTER;

public:
	UINT _id;
	SHADER_TYPE type;
	std::string path;

	std::vector<ID3D11Buffer*> buffers;

	Shader()
	{
		_id = ID_COUNTER++;
	}



	Shader(ID3D11Device* dev, std::string path, std::vector<D3D11_BUFFER_DESC>& descriptions)
	{
		//this feels horrible but whatever... will do it better once it works I guess
		for (int i = 0; i < descriptions.size(); ++i)
		{
			buffers.push_back(nullptr);
			if (FAILED(dev->CreateBuffer(&descriptions[i], NULL, &buffers.back())))
			{
				exit(599);	
			}
		}
		_id = ID_COUNTER++;
	}
};
UINT Shader::ID_COUNTER;


class VertexShader : public Shader
{
public:
	ID3D11VertexShader* _vShader;
	ID3D11InputLayout* _layout;
	
	//std::string strLayout; could be a bitmask really... and not required yet
};



class PixelShader : public Shader
{
public:
	ID3D11PixelShader* _pShader;
	ID3D11SamplerState* _sState;
};