#pragma once
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


namespace ShRef
{
	struct SRCBufferVar
	{
		std::string name;
		uint16_t length{};
		uint16_t offset{};
	};

	struct SRCBuffer
	{
		std::string name;
		uint8_t boundAt{};
		uint16_t size{};
		std::vector<SRCBufferVar> _vars;
	};

	struct SRTexture
	{
		std::string name;
		uint8_t boundAt{};
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

	ID3D11Device* _device{};

	void outputError(ID3DBlob* errorMessage, WCHAR shaderFilename, const std::wstring& filePath) const;

public:

	ShaderCompiler() : _device(nullptr) {};
	ShaderCompiler(ID3D11Device* device) : _device(device) {}

	void init(ID3D11Device* device);

	bool compileVS(const std::wstring& filePath, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inLay, ID3D11VertexShader*& vertexShader, ID3D11InputLayout*& layout) const;
	bool compilePS(const std::wstring& filePath, ID3D11PixelShader*& pixelShader, ShRef::SRShaderMetadata* shMetaData = nullptr) const;
	bool compileGS(const std::wstring& filePath, ID3D11GeometryShader*& geometryShader) const;

	ID3D11VertexShader*		blobToVS(ID3DBlob* shaderBlob) const;
	ID3D11PixelShader*		blobToPS(ID3DBlob* shaderBlob) const;
	ID3D11GeometryShader*	blobToGS(ID3DBlob* shaderBlob) const;

	[[nodiscard]] ID3DBlob* compileToBlob(const std::wstring& filePath, const char* shaderModel) const;
	inline static void PersistBlob(const std::wstring & filePath, ID3DBlob* blob);
	ID3DBlob* loadBlobFromFile(const std::wstring& filePath) const;

	static bool reflect(ID3DBlob* shaderBuffer, ShRef::SRShaderMetadata& shMetaData);

	ID3D11Device* getDevice() const { return _device; }
};