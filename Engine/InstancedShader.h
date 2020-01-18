#pragma once
#include "ShaderLight.h"

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
	std::vector<InstanceData> _instanceData;

public:
	InstancedShader();
	~InstancedShader();

	virtual bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc, unsigned int instanceBufferSizeInElements);
	virtual bool SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const Camera& cam, const PointLight& pLight, float dTime);
	bool UpdateInstanceData(const std::vector<InstanceData>& instanceData);
	void ReleaseShaderParameters(ID3D11DeviceContext* deviceContext);

	ID3D11Buffer* _instanceBuffer;
	unsigned int _instanceCount;
};