#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>

#define DECIMATE(x) if(x) { x->Release(); x = nullptr; }

class ShaderCompiler
{
private:

	HWND* _hwnd;
	ID3D11Device* _device;

public:

	ShaderCompiler() {}



	void ShaderCompiler::init(HWND* hwnd, ID3D11Device* device)
	{
		_hwnd = hwnd;
		_device = device;
	}



	bool compileVS(const std::wstring& filePath, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inLay, ID3D11VertexShader*& vertexShader, ID3D11InputLayout*& layout) const
	{
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* shaderBuffer = nullptr;

		if (FAILED(D3DCompileFromFile(filePath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &shaderBuffer, &errorMessage)))
		{
			outputError(errorMessage, *_hwnd, *(filePath.c_str()), filePath);
			return false;
		}

		// Create the vertex shader from the buffer.
		if (FAILED(_device->CreateVertexShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &vertexShader)))
		{
			MessageBox(*_hwnd, filePath.c_str(), L"Failed to create vertex shader.", MB_OK);
			return false;
		}
		
		// Create the layout related to the vertex shader.
		if (FAILED(_device->CreateInputLayout(inLay.data(), inLay.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &layout)))
		{
			MessageBox(*_hwnd, filePath.c_str(), L"Failed to create vertex input layout.", MB_OK);
			return false;
		}

		// Release the shader buffer since it's no longer needed
		shaderBuffer->Release();

		return true;
	}



	bool compilePS(const std::wstring& filePath, ID3D11PixelShader*& pixelShader) const
	{
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* shaderBuffer = nullptr;

		if (FAILED(D3DCompileFromFile(filePath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &shaderBuffer, &errorMessage)))
		{	
			outputError(errorMessage, *_hwnd, *(filePath.c_str()), filePath);
			return false;
		}

		if (FAILED(_device->CreatePixelShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &pixelShader)))
		{
			MessageBox(*_hwnd, filePath.c_str(), L"Failed to create pixel shader.", MB_OK);
			return false;
		}

		shaderBuffer->Release();

		return true;
	}



	bool compileGS(const std::wstring& filePath, ID3D11GeometryShader*& geometryShader) const
	{
		ID3D10Blob* errorMessage = nullptr;
		ID3D10Blob* shaderBuffer = nullptr;

		if (FAILED(D3DCompileFromFile(filePath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &shaderBuffer, &errorMessage)))
		{
			outputError(errorMessage, *_hwnd, *(filePath.c_str()), filePath);
			return false;
		}

		if (FAILED(_device->CreateGeometryShader(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), NULL, &geometryShader)))
		{
			MessageBox(*_hwnd, filePath.c_str(), L"Failed to create geometry shader.", MB_OK);
			return false;
		}
			

		shaderBuffer->Release();

		return true;
	}



	bool reflect(ID3D10Blob* shaderBuffer)
	{
		ID3D11ShaderReflection* reflection = NULL;
		if (FAILED(D3DReflect(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection)))
		{
			return false;
		}

		D3D11_SHADER_DESC desc;
		reflection->GetDesc(&desc);

		//Find all constant buffers
		for (unsigned int i = 0; i < desc.ConstantBuffers; ++i)
		{
			unsigned int register_index = 0;
			ID3D11ShaderReflectionConstantBuffer* buffer = NULL;
			buffer = reflection->GetConstantBufferByIndex(i);

			D3D11_SHADER_BUFFER_DESC bdesc;
			buffer->GetDesc(&bdesc);

			//get variables out of cbuffer
			for (unsigned int j = 0; j < bdesc.Variables; ++j)
			{
				ID3D11ShaderReflectionVariable* variable = NULL;
				variable = buffer->GetVariableByIndex(j);

				D3D11_SHADER_VARIABLE_DESC vdesc;
				variable->GetDesc(&vdesc);

				/*ShaderVariable* shadervariable = new ShaderVariable();
				shadervariable->name = Engine::String.ConvertToWideStr(vdesc.Name);
				shadervariable->length = vdesc.Size;
				shadervariable->offset = vdesc.StartOffset;
				mSize += vdesc.Size;
				mVariables.push_back(shadervariable);*/
			}

			for (unsigned int k = 0; k < desc.BoundResources; ++k)
			{
				D3D11_SHADER_INPUT_BIND_DESC ibdesc;
				reflection->GetResourceBindingDesc(k, &ibdesc);

				if (!strcmp(ibdesc.Name, bdesc.Name))
					register_index = ibdesc.BindPoint;
			}

			//Add constant buffers straight outta here... should be easier than having to code it all the time!
		}
	}



	bool createSamplerState(const D3D11_SAMPLER_DESC& samplerDesc, ID3D11SamplerState*& sampleState) const
	{
		if (FAILED(_device->CreateSamplerState(&samplerDesc, &sampleState)))
		{
			MessageBoxA(*_hwnd, std::string(__FILE__).c_str(), "Failed to create sampler state shader.", MB_OK);
			return false;
		}
		return true;
	}



	bool createConstantBuffer(const D3D11_BUFFER_DESC& desc, ID3D11Buffer*& buffer) const
	{
		if (FAILED(_device->CreateBuffer(&desc, NULL, &buffer)))
		{
			MessageBoxA(*_hwnd, std::string(__FILE__).c_str(), "Failed to create constant buffer shader.", MB_OK);
			return false;
		}
		return true;
	}



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


	//ZeroMemory(&sbSamplerDesc, sizeof(sbSamplerDesc));
	//sbSamplerDesc = { D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 1, D3D11_COMPARISON_ALWAYS, 0, 0, 0, 0, 0, D3D11_FLOAT32_MAX };
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

private:

	void ShaderCompiler::outputError(ID3D10Blob* errorMessage, HWND hwnd, WCHAR shaderFilename, const std::wstring& filePath) const
	{
		if (!errorMessage)
		{
			MessageBox(*_hwnd, filePath.c_str(), L"Shader file not found.", MB_OK);
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