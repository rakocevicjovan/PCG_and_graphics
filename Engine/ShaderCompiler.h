#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <string>
#include <vector>


#define DECIMATE(x) if(x) { x->Release(); x = nullptr; }


namespace ShRef
{
	struct SRCBufferVar
	{
		std::string name;
		uint16_t length;
		uint16_t offset;
	};

	struct SRCBuffer
	{
		std::string name;
		uint8_t boundAt;
		uint16_t size;
		std::vector<SRCBufferVar> _vars;
	};

	struct SRTexture
	{
		std::string name;
		uint8_t boundAt;
	};

	struct SRShaderMetadata
	{
		std::vector<SRCBuffer> _cBuffers;
		std::vector<SRTexture> _textures;
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
	bool compilePS(const std::wstring& filePath, ID3D11PixelShader*& pixelShader, ShRef::SRShaderMetadata* shMetaData = nullptr) const;
	bool compileGS(const std::wstring& filePath, ID3D11GeometryShader*& geometryShader) const;

	static bool reflect(ID3D10Blob* shaderBuffer, ShRef::SRShaderMetadata& shMetaData);

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
};