#include "ShaderCompiler.h"
#include "Shader.h"
#include <unordered_map>

class ShaderCache
{
	ShaderCompiler* _shCompiler;
	std::unordered_map<std::string, VertexShader*> _vsMap;
	std::unordered_map<std::string, PixelShader*> _psMap;

	//@TODO obviously, this is to be replaced, sooner the better
	void createAllShadersBecauseIAmTooLazyToMakeThisDataDriven();

public:
	
	void init(ShaderCompiler* shc);
	
	VertexShader* getVertShader(const std::string& name);
	PixelShader* getPixShader(const std::string& name);

	bool addVertShader(const std::string& name, VertexShader* vs);
	bool addPixShader(const std::string& name, PixelShader* ps);
};