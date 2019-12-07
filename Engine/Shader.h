#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <string>


enum class SHADER_TYPE { VS, GS, PS};


class Shader
{
public:
	std::string path;
	SHADER_TYPE type;
};



class VertexShader : public Shader
{
public:
	ID3D11InputLayout* _layout;
	ID3D11VertexShader* _vertexShader;


};