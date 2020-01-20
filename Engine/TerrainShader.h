#include "Biome.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"


class TerrainShader
{
protected:
	std::vector<std::wstring> _filePaths;
	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };

public:
	TerrainShader();
	~TerrainShader();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	void SetShaderParameters(ID3D11DeviceContext* dc, const SMatrix& mt, const SVec3& camPos, const PointLight& pointLight);
	void ReleaseShaderParameters(ID3D11DeviceContext*);

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _lightBuffer;

	RenderFormat renderFormat;		//belongs to vertex data, shouldn't be here

	unsigned int texturesAdded = 0;	//not robust

	bool done = false;
};