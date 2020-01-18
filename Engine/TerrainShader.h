#include "Biome.h"
#include "ShaderDataStructs.h"

class Camera;

class TerrainShader
{
public:
	TerrainShader();
	~TerrainShader();

	bool Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);

	void SetShaderParameters(ID3D11DeviceContext* dc, const SMatrix& mt, const Camera& cam, const PointLight& pointLight, float deltaTime);

	bool done = false;
};