#pragma once
#include <d3d11_4.h>
#include <vector>
#include <array>

// Not going to bother with cross-API versions yet
class Sampler
{
public:

	ID3D11SamplerState* _dxSampler;


	Sampler() : _dxSampler(nullptr) {}



	inline static Sampler createSampler(ID3D11Device* device, const D3D11_SAMPLER_DESC* samplerDesc)
	{
		Sampler result;
		setUp(device, samplerDesc, result._dxSampler);
		return result;
	}



	inline static void setUp(ID3D11Device* device, const D3D11_SAMPLER_DESC* samplerDesc, ID3D11SamplerState*& samplerState)
	{
		if (FAILED(device->CreateSamplerState(samplerDesc, &samplerState)))
		{
			OutputDebugStringA("Failed to create sampler.");
			exit(1001);
		}
	}



	inline static D3D11_SAMPLER_DESC createSamplerDesc(
		D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_COMPARISON_FUNC comparisonFunc = D3D11_COMPARISON_ALWAYS,
		FLOAT minLOD = 0.f,
		FLOAT maxLOD = D3D11_FLOAT32_MAX,
		D3D11_TEXTURE_ADDRESS_MODE addressU = D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MODE addressV = D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MODE addressW = D3D11_TEXTURE_ADDRESS_WRAP,
		FLOAT mipLODBias = 0.0f,
		UINT maxAnisotropy = 1.0f,
		std::array<float, 4> borderColor = { 0.f, 0.f, 0.f, 0.f })
	{
		D3D11_SAMPLER_DESC sDesc = {};
		sDesc.Filter = filter;
		sDesc.AddressU = addressU;
		sDesc.AddressV = addressV;
		sDesc.AddressW = addressW;
		sDesc.MipLODBias = mipLODBias;
		sDesc.MaxAnisotropy = maxAnisotropy;
		sDesc.ComparisonFunc = comparisonFunc;
		memcpy(sDesc.BorderColor, borderColor.data(), sizeof(sDesc.BorderColor));
		sDesc.MinLOD = minLOD;
		sDesc.MaxLOD = maxLOD;
		return sDesc;
	}
};