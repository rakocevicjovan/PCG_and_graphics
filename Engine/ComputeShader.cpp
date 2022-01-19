#include "pch.h"
#include "ComputeShader.h"

bool ComputeShader::createFromFile(ID3D11Device* device, const std::wstring& path)
{
	// Reminder for constants
	D3D11_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
	D3D11_PS_CS_UAV_REGISTER_COUNT;
	D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	// Prefer higher CS shader profile when possible as CS 5.0 provides better performance on 11-class hardware.
	//LPCSTR profile = (shc.getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
	// However, I am only building for DX 11.* so it doesn't matter right now
	LPCSTR profile = "cs_5_0";

	/*
	// To add defines to D3DCompileFromFile (path, defines, ...)
	const D3D_SHADER_MACRO defines[] = { "EXAMPLE_DEFINE", "1", NULL, NULL };
	*/

	ID3DBlob* shaderBlob{};
	ID3DBlob* errorBlob{};
	HRESULT hr{ NULL };

	hr = D3DCompileFromFile(path.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", profile, flags, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr))
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());

	hr = device->CreateComputeShader(shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		NULL, _shaderPtr.GetAddressOf());

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


void ComputeShader::execute(ID3D11DeviceContext* context, std::array<uint32_t, 3> xyz,
	std::vector<ID3D11ShaderResourceView*>& srvs,
	std::vector<ID3D11UnorderedAccessView*>& uavs)
{
	context->CSSetShader(_shaderPtr.Get(), NULL, 0);
	context->CSSetShaderResources(0, srvs.size(), srvs.data());
	context->CSSetUnorderedAccessViews(0, uavs.size(), uavs.data(), NULL);

	// Specifies how many thread GROUPS to launch
	context->Dispatch(xyz[0], xyz[1], xyz[2]);

	// This is how it's "unbound" - as usual, just bind nulls. 
	//context->CSSetShader(NULL, NULL, 0);
	//context->CSSetShaderResources(0, some_null_srvs.size(), some_null_srvs.data());
	//context->CSSetUnorderedAccessViews(0, some_null_uavs.size(), some_null_uavs.data(), NULL);
}
