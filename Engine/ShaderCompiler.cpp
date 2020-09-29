#include "ShaderCompiler.h"
#include "Logger.h"



void ShaderCompiler::ShaderCompiler::init(ID3D11Device* device)
{
	_device = device;
}



bool ShaderCompiler::compileVS(const std::wstring& filePath, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inLay, 
	ID3D11VertexShader*& vertexShader, ID3D11InputLayout*& layout) const
{
	ID3DBlob* shaderBuffer = compileToBlob(filePath, "vs_5_0");
	vertexShader = blobToVS(shaderBuffer);

	// Create the layout related to the vertex shader.
	if (FAILED(_device->CreateInputLayout(inLay.data(), inLay.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &layout)))
	{
		OutputDebugStringA("Failed to create vertex input layout.");
		return false;
	}

	// Release the shader buffer since it's no longer needed
	shaderBuffer->Release();

	return vertexShader;
}



bool ShaderCompiler::compilePS(const std::wstring& filePath, ID3D11PixelShader*& pixelShader, ShRef::SRShaderMetadata* shMetaData) const
{
	ID3DBlob* shaderBuffer = compileToBlob(filePath, "ps_5_0");
	pixelShader = blobToPS(shaderBuffer);

	//not used just yet... but tested, works great
	//if(shMetaData != nullptr)
	//reflect(shaderBuffer, *shMetaData);

	shaderBuffer->Release();
	return pixelShader;
}



bool ShaderCompiler::compileGS(const std::wstring& filePath, ID3D11GeometryShader*& geometryShader) const
{
	ID3DBlob* shaderBuffer = compileToBlob(filePath, "gs_5_0");
	geometryShader = blobToGS(shaderBuffer);
	shaderBuffer->Release();
	return geometryShader;
}



ID3D11VertexShader* ShaderCompiler::blobToVS(ID3DBlob* shaderBlob) const
{
	ID3D11VertexShader* result;
	if (FAILED(_device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &result)))
	{
		OutputDebugStringA("Failed to create vertex shader.");
		return nullptr;
	}
	return result;
}



ID3D11PixelShader* ShaderCompiler::blobToPS(ID3DBlob* shaderBlob) const
{
	ID3D11PixelShader* result;
	if (FAILED(_device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &result)))
	{
		OutputDebugStringA("Failed to create pixel shader.");
		return nullptr;
	}
	return result;
}



ID3D11GeometryShader* ShaderCompiler::blobToGS(ID3DBlob* shaderBlob) const
{
	ID3D11GeometryShader* result;
	if (FAILED(_device->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &result)))
	{
		OutputDebugStringA("Failed to create geometry shader.");
		return nullptr;
	}
	return result;
}


// Remember to release it after use
ID3DBlob* ShaderCompiler::compileToBlob(const std::wstring& filePath, const char* shaderModel) const
{
	ID3DBlob* shaderBlob;
	ID3DBlob* errorMessage;

	//D3DCOMPILE_WARNINGS_ARE_ERRORS
	if (FAILED(D3DCompileFromFile(filePath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", shaderModel, D3D10_SHADER_ENABLE_STRICTNESS, 0, &shaderBlob, &errorMessage)))
	{
		outputError(errorMessage, *(filePath.c_str()), filePath);
		return false;
	}

	return shaderBlob;
}



inline void ShaderCompiler::PersistBlob(const std::wstring & filePath, ID3DBlob * blob)
{
	D3DWriteBlobToFile(blob, filePath.c_str(), true);
}



ID3DBlob* ShaderCompiler::loadBlobFromFile(const std::wstring& filePath) const
{
	ID3DBlob* shaderBlob;
	D3DReadFileToBlob(filePath.c_str(), &shaderBlob);
	return shaderBlob;
}



bool ShaderCompiler::reflect(ID3DBlob* shaderBuffer, ShRef::SRShaderMetadata& shMetaData)
{
	ID3D11ShaderReflection* reflection = NULL;

	if (FAILED(D3DReflect(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection)))
		return false;

	D3D11_SHADER_DESC desc;
	reflection->GetDesc(&desc);

	// Find all constant buffer metadata
	// Could be useful to create them from this data dynamically but for now I'll only keep it stored
	
	shMetaData._cBuffers.reserve(desc.ConstantBuffers);

	for (unsigned int i = 0; i < desc.ConstantBuffers; ++i)
	{
		ID3D11ShaderReflectionConstantBuffer* buffer = NULL;
		buffer = reflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC bDesc;
		buffer->GetDesc(&bDesc);

		ShRef::SRCBuffer cBuffer;
		cBuffer.name = bDesc.Name;
		cBuffer.size = bDesc.Size;

		// Get variable descriptions out of cbuffer
		
		cBuffer._vars.reserve(bDesc.Variables);

		for (unsigned int j = 0; j < bDesc.Variables; ++j)
		{
			ID3D11ShaderReflectionVariable* variable = NULL;
			variable = buffer->GetVariableByIndex(j);

			D3D11_SHADER_VARIABLE_DESC vdesc;
			variable->GetDesc(&vdesc);

			ShRef::SRCBufferVar srcb;
			srcb.name = vdesc.Name;
			srcb.length = vdesc.Size;
			srcb.offset = vdesc.StartOffset;
			cBuffer._vars.push_back(srcb);
		}

		// Find the bind point of the shader
		for (unsigned int k = 0; k < desc.BoundResources; ++k)
		{
			D3D11_SHADER_INPUT_BIND_DESC IBDesc;
			reflection->GetResourceBindingDesc(k, &IBDesc);

			if (!strcmp(IBDesc.Name, bDesc.Name))	//strcmp returns 0 when strings match
				cBuffer.boundAt = IBDesc.BindPoint;
		}
		shMetaData._cBuffers.push_back(cBuffer);
	}


	// Find all texture metadata
	for (unsigned int k = 0; k < desc.BoundResources; ++k)
	{
		D3D11_SHADER_INPUT_BIND_DESC ibdesc;
		reflection->GetResourceBindingDesc(k, &ibdesc);

		if (ibdesc.Type == D3D_SHADER_INPUT_TYPE::D3D10_SIT_TEXTURE)
		{
			ShRef::SRTexture t;
			t.name = ibdesc.Name;
			t.boundAt = ibdesc.BindPoint;
			shMetaData._textures.push_back(t);
		}
	}
}



void ShaderCompiler::outputError(ID3DBlob* errBlob, WCHAR shaderFilename, const std::wstring& filePath) const
{
	std::string filePathNarrow(filePath.begin(), filePath.end());
	if (!errBlob)
	{
		std::string errMsg = "Shader file not found: " + filePathNarrow;
		OutputDebugStringA(errMsg.c_str());
		__debugbreak();
		return;
	}

	FileUtils::writeAllBytes("shader-error.txt", errBlob->GetBufferPointer(), errBlob->GetBufferSize());

	errBlob->Release();
	errBlob = nullptr;

	std::string errMsg = "Error compiling shader.  Check shader-error.txt for message.";
	__debugbreak();
	OutputDebugStringA(errMsg.c_str());
}