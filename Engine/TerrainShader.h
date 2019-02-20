#include "ShaderBase.h"
#include "Biome.h"

class TerrainShader : public ShaderBase
{
	

public:
	TerrainShader();
	~TerrainShader();

	bool Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);

	bool SetShaderParameters(SPBase* spb);


};