#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <string>
#include <vector>


#define DECIMATE(x) if(x) { x->Release(); x = nullptr; }


namespace ShRef
{
	struct CBufferVar
	{
		std::string name;
		uint16_t length;
		uint16_t offset;
	};

	struct CBuffer
	{
		std::string name;
		uint8_t boundAt;
		uint16_t size;
		std::vector<CBufferVar> _vars;
	};

	struct Texture
	{
		std::string name;
		uint8_t boundAt;
	};

	struct ShaderMetadata
	{
		std::vector<CBuffer> _cBuffers;
		std::vector<Texture> _textures;
	};
}



class ShaderCompiler
{
private:

	HWND* _hwnd;
	ID3D11Device* _device;

	void ShaderCompiler::outputError(ID3D10Blob* errorMessage, HWND hwnd, WCHAR shaderFilename, const std::wstring& filePath) const;

public:

	ShaderCompiler();

	void ShaderCompiler::init(HWND* hwnd, ID3D11Device* device);

	bool compileVS(const std::wstring& filePath, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inLay, ID3D11VertexShader*& vertexShader, ID3D11InputLayout*& layout) const;
	bool compilePS(const std::wstring& filePath, ID3D11PixelShader*& pixelShader, ShRef::ShaderMetadata* shMetaData = nullptr) const;
	bool compileGS(const std::wstring& filePath, ID3D11GeometryShader*& geometryShader) const;

	static bool reflect(ID3D10Blob* shaderBuffer, ShRef::ShaderMetadata& shMetaData);

	bool createSamplerState(const D3D11_SAMPLER_DESC& samplerDesc, ID3D11SamplerState*& sampleState) const;
	bool createConstantBuffer(const D3D11_BUFFER_DESC& desc, ID3D11Buffer*& buffer) const;


	ID3D11Device* getDevice() const { return _device; }


	inline static D3D11_BUFFER_DESC createBufferDesc(
		UINT byteWidth,
		D3D11_USAGE usage = D3D11_USAGE_DYNAMIC,
		D3D11_BIND_FLAG binding = D3D11_BIND_CONSTANT_BUFFER,
		D3D11_CPU_ACCESS_FLAG cpuAccessFlag = D3D11_CPU_ACCESS_WRITE,
		UINT miscFlag = 0u,
		UINT stride = 0u)
	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = byteWidth;
		cbDesc.Usage = usage;
		cbDesc.BindFlags = binding;
		cbDesc.CPUAccessFlags = cpuAccessFlag;
		cbDesc.MiscFlags = miscFlag;
		cbDesc.StructureByteStride = stride;
		return cbDesc;
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
		std::vector<FLOAT> borderColor = { 0.f, 0.f, 0.f, 0.f })
	{
		D3D11_SAMPLER_DESC sDesc;
		ZeroMemory(&sDesc, sizeof(D3D11_SAMPLER_DESC));
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