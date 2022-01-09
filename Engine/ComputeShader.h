#pragma once
#include "ShaderCompiler.h"
#include "SBuffer.h"

class ComputeShader
{
protected:

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> _shaderPtr;

public:

	ComputeShader() : _shaderPtr(nullptr) {}

	bool createFromFile(ID3D11Device* device, const std::wstring& path);

	void ComputeShader::execute(ID3D11DeviceContext* context, std::array<uint32_t, 3> xyz,
		std::vector<ID3D11ShaderResourceView*>& srvs,
		std::vector<ID3D11UnorderedAccessView*>& uavs);
};