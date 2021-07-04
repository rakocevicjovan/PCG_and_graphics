#pragma once
#include "Shader.h"

class ShaderCache
{
	std::unordered_map<std::string, VertexShader*> _vsMap;
	std::unordered_map<std::string, PixelShader*> _psMap;

public:
	void init();
	
	VertexShader* getVertShader(const std::string& name);
	PixelShader* getPixShader(const std::string& name);

	bool addVertShader(const std::string& name, VertexShader* vs);
	bool addPixShader(const std::string& name, PixelShader* ps);

	//@TODO obviously, this is to be replaced, sooner the better
	void createAllShadersBecauseIAmTooLazyToMakeThisDataDriven(ShaderCompiler* shc);
};