#pragma once
#include "ShaderCompiler.h"
#include "SBuffer.h"

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
		const D3D_SHADER_MACRO defines[] = { "EXAMPLE_DEFINE", "1", NULL, NULL };*/

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


	void ComputeShader::execute(ID3D11DeviceContext* context, ID3D11ComputeShader* cs, UINT xyz[3],
		std::vector<ID3D11ShaderResourceView*>& srvs,
		std::vector<ID3D11UnorderedAccessView*>& uavs)
	{
		// Having to allocate on the heap for this is absolute bollocks but leaving it bound seems bad as well...
		std::vector<ID3D11ShaderResourceView*> srvNulls(srvs.size(), NULL);
		std::vector<ID3D11UnorderedAccessView*> uavNulls(uavs.size(), NULL);

		context->CSSetShader(cs, NULL, 0);
		context->CSSetShaderResources(0, srvs.size(), srvs.data());
		context->CSSetUnorderedAccessViews(0, uavs.size(), uavs.data(), NULL);

		// Specifies how many thread GROUPS to launch
		context->Dispatch(xyz[0], xyz[1], xyz[2]);

		context->CSSetShader(NULL, NULL, 0);
		context->CSSetShaderResources(0, srvNulls.size(), srvNulls.data());
		context->CSSetUnorderedAccessViews(0, uavNulls.size(), uavNulls.data(), NULL);

	}

};