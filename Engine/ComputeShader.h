#pragma once
#include "ShaderCompiler.h"

class ComputeShader
{
protected:

	ID3D11ComputeShader* _shaderPtr;


public:

	ComputeShader() : _shaderPtr(nullptr) {}


	bool ComputeShader::createFromFile(ShaderCompiler& shc, const std::wstring& path)
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		// Prefer higher CS shader profile when possible as CS 5.0 provides better performance on 11-class hardware.
		//LPCSTR profile = (shc.getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
		// However, I am only building for DX 11.* so it doesn't matter right now
		LPCSTR profile = "cs_5_0";

		/*
		// also add defines to D3DCompileFromFile (path, defines, ...)
		const D3D_SHADER_MACRO defines[] =
		{
			"EXAMPLE_DEFINE", "1",
			NULL, NULL
		};*/

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = NULL;


		hr = D3DCompileFromFile(path.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", profile, flags, 0, &shaderBlob, &errorBlob);
		
		if (FAILED(hr))
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());


		hr = shc.getDevice()->CreateComputeShader(shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize(),
			NULL, &_shaderPtr);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			if (shaderBlob)
				shaderBlob->Release();

			return false;
		}

		return true;
	}

};