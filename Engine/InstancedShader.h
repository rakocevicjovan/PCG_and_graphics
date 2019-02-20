#pragma once
#include "ShaderBase.h"

struct BaseInstanceData {};

struct InstanceData : public BaseInstanceData
{
	SMatrix _m;

	InstanceData() {};
	InstanceData(SMatrix m) { _m = m; }
};

class InstancedShader : public ShaderBase
{
protected:
	std::vector<InstanceData> _instanceData;

public:
	InstancedShader();
	~InstancedShader();

	virtual bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc, unsigned int instanceBufferSizeInElements);
	virtual bool SetShaderParameters(SPBase* spb);
	bool UpdateInstanceData(const std::vector<InstanceData>& instanceData);

	ID3D11Buffer* _instanceBuffer;
	unsigned int _instanceCount;
};