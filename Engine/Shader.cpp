#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Shader.h"
#include "Sampler.h"
#include "CBuffer.h"
#include <d3d11.h>



Shader::Shader(const ShaderCompiler& shc, const std::wstring& path, const std::vector<D3D11_BUFFER_DESC>& descriptions)
{
	_path = path;
	_cbuffers.resize(descriptions.size());

	for (int i = 0; i < descriptions.size(); ++i)
	{
		CBuffer::createBuffer(shc.getDevice(), descriptions[i], _cbuffers[i]._cbPtr);
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
	: Shader(shc, path, descriptions)
{
	_type = SHADER_TYPE::VS;
	shc.compileVS(path, inputLayoutDesc, _vsPtr, _layout);
}



void VertexShader::setBuffers(ID3D11DeviceContext* cont)
{
	for (int i = 0; i < _cbuffers.size(); ++i)
		cont->VSSetConstantBuffers(_cbuffers[i]._metaData._slot, 1, &_cbuffers[i]._cbPtr);

	// THIS CAN'T WORK UNLESS THEY ARE CONTIGUOUS IN MEMORY BUT IT'S FASTER! @TODO REORGANIZE THAT PART
	//if (_cbuffers.size() > 0)
		//cont->VSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]._cbPtr);
}



PixelShader::PixelShader(
	const ShaderCompiler& shc,
	const std::wstring& path,
	const std::vector<D3D11_SAMPLER_DESC>& samplerDescs,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(shc, path, descriptions)
{
	_type = SHADER_TYPE::PS;
	shc.compilePS(path, _psPtr, &_refShMetaData);
	
	UINT numSamplers = samplerDescs.size();
	_sStates.resize(numSamplers);

	for (UINT i = 0; i < numSamplers; ++i)
	{
		Sampler::setUp(shc.getDevice(), &samplerDescs[i], _sStates[i]);
	}
}



void PixelShader::setBuffers(ID3D11DeviceContext* cont)
{
	for (int i = 0; i < _cbuffers.size(); ++i)
		cont->PSSetConstantBuffers(_cbuffers[i]._metaData._slot, 1, &_cbuffers[i]._cbPtr);

	// THIS CAN'T WORK UNLESS THEY ARE CONTIGUOUS IN MEMORY BUT IT'S FASTER! @TODO REORGANIZE THAT PART
	//if(_cbuffers.size() > 0)
		//cont->PSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]._cbPtr);
}