#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>



class ShaderCompiler
{
private:

	HWND& _hwnd;
	ID3D11Device*& _device;

public:

	ShaderCompiler(HWND& hwnd, ID3D11Device*& device) : _hwnd(hwnd), _device(device) {}



	bool compileVS(const std::wstring& filePath, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inLay, ID3D11VertexShader*& vertexShader, ID3D11InputLayout*& layout)
	{
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* shaderBuffer = nullptr;

		if (FAILED(D3DCompileFromFile(filePath.c_str(), NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &shaderBuffer, &errorMessage)))
		{
			outputError(errorMessage, _hwnd, *(filePath.c_str()), filePath);
			return false;
		}

		// Create the vertex shader from the buffer.
		if (FAILED(_device->CreateVertexShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &vertexShader)))
		{
			MessageBox(_hwnd, filePath.c_str(), L"Failed to create vertex shader.", MB_OK);
			return false;
		}
		
		// Create the layout related to the vertex shader.
		if (FAILED(_device->CreateInputLayout(inLay.data(), inLay.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &layout)))
		{
			MessageBox(_hwnd, filePath.c_str(), L"Failed to create vertex input layout.", MB_OK);
			return false;
		}

		// Release the shader buffer since it's no longer needed
		shaderBuffer->Release();

		return true;
	}



	bool compilePS(const std::wstring& filePath, ID3D11PixelShader*& pixelShader)
	{
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* shaderBuffer = nullptr;

		if (FAILED(D3DCompileFromFile(filePath.c_str(), NULL, NULL, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &shaderBuffer, &errorMessage)))
		{	
			outputError(errorMessage, _hwnd, *(filePath.c_str()), filePath);
			return false;
		}

		if (FAILED(_device->CreatePixelShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &pixelShader)))
		{
			MessageBox(_hwnd, filePath.c_str(), L"Failed to create pixel shader.", MB_OK);
			return false;
		}

		shaderBuffer->Release();

		return true;
	}



	bool compileGS(const std::wstring& filePath, ID3D11GeometryShader*& geometryShader)
	{
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* shaderBuffer = nullptr;

		if (FAILED(D3DCompileFromFile(filePath.c_str(), NULL, NULL, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &shaderBuffer, &errorMessage)))
		{
			outputError(errorMessage, _hwnd, *(filePath.c_str()), filePath);
			return false;
		}

		if (FAILED(_device->CreateGeometryShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &geometryShader)))
		{
			MessageBox(_hwnd, filePath.c_str(), L"Failed to create geometry shader.", MB_OK);
			return false;
		}
			

		shaderBuffer->Release();

		return true;
	}



	bool createSamplerState(const D3D11_SAMPLER_DESC& samplerDesc, ID3D11SamplerState*& sampleState)
	{
		if (FAILED(_device->CreateSamplerState(&samplerDesc, &sampleState)))
		{
			MessageBoxA(_hwnd, std::string(__FILE__).c_str(), "Failed to create sampler state shader.", MB_OK);
			return false;
		}
		return true;
	}



	bool createConstantBuffer(const D3D11_BUFFER_DESC& desc, ID3D11Buffer*& buffer)
	{
		if (FAILED(_device->CreateBuffer(&desc, NULL, &buffer)))
		{
			MessageBoxA(_hwnd, std::string(__FILE__).c_str(), "Failed to create constant buffer shader.", MB_OK);
			return false;
		}
		return true;
	}



	void createMaterial()
	{

	}

private:

	void ShaderCompiler::outputError(ID3D10Blob* errorMessage, HWND hwnd, WCHAR shaderFilename, const std::wstring& filePath)
	{
		if (!errorMessage)
		{
			MessageBox(_hwnd, filePath.c_str(), L"Shader file not found.", MB_OK);
			return;
		}

		char* compileErrors;
		unsigned long bufferSize, i;
		std::ofstream fout;

		compileErrors = (char*)(errorMessage->GetBufferPointer());
		bufferSize = errorMessage->GetBufferSize();
		fout.open("../Shaders/shader-error.txt");
		for (i = 0; i < bufferSize; i++)
			fout << compileErrors[i];

		fout.close();

		errorMessage->Release();
		errorMessage = nullptr;

		MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", &shaderFilename, MB_OK);
	}
};