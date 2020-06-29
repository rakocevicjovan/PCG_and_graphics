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


	ID3D11Device* getDevice() const { return _device; }
};