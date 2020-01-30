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
		shc.createConstantBuffer(descriptions[i], _cbuffers[i]._cbPtr);
	}
}



bool Shader::updateCBufferDirectly(ID3D11DeviceContext* cont, void* data, uint8_t index)
{
	return CBuffer::updateWholeBuffer(cont, _cbuffers[index]._cbPtr, data, _cbuffers[index]._metaData._size);
}



VertexShader::VertexShader(
	const ShaderCompiler& shc,
	const std::wstring& path,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(shc , path, descriptions)
{
	_type = SHADER_TYPE::VS;
	shc.compileVS(path, inputLayoutDesc,_vsPtr, _layout);
}



void VertexShader::setBuffers(ID3D11DeviceContext* cont)
{
	//for (int i = 0; i < _cbuffers.size(); ++i)
		//cont->VSSetConstantBuffers(i, 1, &_cbuffers[i]);
	if (_cbuffers.size() > 0)
		cont->VSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]._cbPtr);
}



PixelShader::PixelShader(
	const ShaderCompiler& shc,
	const std::wstring& path,
	const D3D11_SAMPLER_DESC& samplerDesc,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(shc, path, descriptions)
{
	_type = SHADER_TYPE::PS;
	shc.compilePS(path, _psPtr);
	shc.createSamplerState(samplerDesc, _sState);
}



void PixelShader::setBuffers(ID3D11DeviceContext* cont)
{
	if(_cbuffers.size() > 0)
		cont->PSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]._cbPtr);
}