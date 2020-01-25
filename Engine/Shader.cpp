#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Shader.h"
#include <d3d11.h>



UINT Shader::ID_COUNTER = 0u;



Shader::Shader(const ShaderCompiler& shc, const std::wstring& path, const std::vector<D3D11_BUFFER_DESC>& descriptions) :_id(ID_COUNTER++)
{
	_path = path;
	_cbuffers.resize(descriptions.size());

	for (int i = 0; i < descriptions.size(); ++i)
	{
		shc.createConstantBuffer(descriptions[i], _cbuffers[i]);
	}
}



bool Shader::updateCBufferDirectly(ID3D11DeviceContext* cont, void* data, uint8_t index)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	if (FAILED(cont->Map(_cbuffers[index], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	void* dataPtr = mappedResource.pData;
	memcpy(dataPtr, data, _bufferMetaData[index]._size);
	cont->Unmap(_cbuffers[index], 0);

	return true;
}



VertexShader::VertexShader(
	const ShaderCompiler& shc,
	const std::wstring& path,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(shc , path, descriptions)
{
	_type = SHADER_TYPE::VS;
	shc.compileVS(path, inputLayoutDesc,_vShader, _layout);
}



void VertexShader::setBuffers(ID3D11DeviceContext* cont)
{
	//for (int i = 0; i < _cbuffers.size(); ++i)
		//cont->VSSetConstantBuffers(i, 1, &_cbuffers[i]);

	cont->VSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]);
}



PixelShader::PixelShader(
	const ShaderCompiler& shc,
	const std::wstring& path,
	const D3D11_SAMPLER_DESC& samplerDesc,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(shc, path, descriptions)
{
	_type = SHADER_TYPE::VS;
	shc.compilePS(path, _pShader);
	shc.createSamplerState(samplerDesc, _sState);
}



void PixelShader::setBuffers(ID3D11DeviceContext* cont)
{
	cont->PSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]);
}