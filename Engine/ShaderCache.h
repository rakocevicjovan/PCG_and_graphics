#pragma once
#include "ShaderCompiler.h"
#include "Shader.h"
#include <unordered_map>

class ShaderCache
{
	ShaderCompiler* _shc;
	std::unordered_map<std::string, VertexShader*> _vsMap;
	std::unordered_map<std::string, PixelShader*> _psMap;

public:
	void init(ShaderCompiler* shCompiler);
	
	VertexShader* getVertShader(const std::string& name);
	PixelShader* getPixShader(const std::string& name);

	bool addVertShader(const std::string& name, VertexShader* vs);
	bool addPixShader(const std::string& name, PixelShader* ps);

	//@TODO obviously, this is to be replaced, sooner the better
	void createAllShadersBecauseIAmTooLazyToMakeThisDataDriven();
};