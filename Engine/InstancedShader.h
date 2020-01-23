#pragma once
#include "Phong.h"

class Camera;

struct InstanceData
{
	SMatrix _m;

	InstanceData() {};
	InstanceData(const SMatrix& m) : _m(m) {}
};



class InstancedShader
{
protected:
	std::vector<std::wstring> _filePaths;
	std::vector<InstanceData> _instanceData;

public:
	InstancedShader();
	~InstancedShader();

	virtual bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc, unsigned int instanceBufferSizeInElements);
	virtual bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const Camera& cam, const PointLight& pLight, float dTime);
	bool UpdateInstanceData(const std::vector<InstanceData>& instanceData);
	void ReleaseShaderParameters(ID3D11DeviceContext* deviceContext);

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _lightBuffer;
	ID3D11Buffer* _instanceBuffer;

	RenderFormat renderFormat;		//should it be here?

	unsigned int texturesAdded = 0;	//not robust...
	
	unsigned int _instanceCount;
};