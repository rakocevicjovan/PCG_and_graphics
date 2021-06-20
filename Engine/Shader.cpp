#include "pch.h"
#include "Shader.h"
#include "Sampler.h"
#include "CBuffer.h"


Shader::Shader(ID3D11Device* device, const std::wstring& path, const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: _path(path)
{
	_cbuffers.resize(descriptions.size());

	for (int i = 0; i < descriptions.size(); ++i)
	{
		_cbuffers[i].init(device, descriptions[i]);
	}
}


void Shader::updateCBufferDirectly(ID3D11DeviceContext* cont, void* data, uint8_t index)
{
	_cbuffers[index].update(cont, data, _cbufferMetaData[index]._size);
}


VertexShader::VertexShader(
	const ShaderCompiler& shc,
	const std::wstring& path,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(shc.getDevice(), path, descriptions)
{
	_type = ShaderType::VS;

	shc.compileVS(path, inputLayoutDesc, *_vsPtr.GetAddressOf(), *_layout.GetAddressOf());
}


VertexShader::VertexShader(
	ID3D11Device* device,
	void* ptr,
	UINT size,
	const std::wstring& path,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inLay,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(device, path, descriptions)
{
	_type = ShaderType::VS;
	if (FAILED(device->CreateVertexShader(ptr, size, NULL, &_vsPtr)))
		__debugbreak();

	if (FAILED(device->CreateInputLayout(inLay.data(), inLay.size(), ptr, size, &_layout)))
		__debugbreak();
}


void VertexShader::setBuffers(ID3D11DeviceContext* cont)
{
	for (int i = 0; i < _cbuffers.size(); ++i)
	{
		_cbuffers[i].bindToVS(cont, (_cbufferMetaData[i]._slot));
	}

	// THIS CAN'T WORK UNLESS THEY ARE CONTIGUOUS IN MEMORY BUT IT'S FASTER! @TODO REORGANIZE THAT PART
	//if (_cbuffers.size() > 0)
		//cont->VSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]._cbPtr);
}



PixelShader::PixelShader(
	const ShaderCompiler& shc,
	const std::wstring& path,
	const std::vector<D3D11_SAMPLER_DESC>& samplerDescs,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(shc.getDevice(), path, descriptions)
{
	_type = ShaderType::PS;
	shc.compilePS(path, *_psPtr.GetAddressOf(), &_refShMetaData);
	
	UINT numSamplers = samplerDescs.size();
	_samplers.resize(numSamplers);

	for (UINT i = 0; i < numSamplers; ++i)
		Sampler::SetUp(shc.getDevice(), &samplerDescs[i], *_samplers[i].GetAddressOf());
}


PixelShader::PixelShader(
	ID3D11Device* device,
	void* ptr,
	UINT size,
	const std::wstring& path,
	const std::vector<D3D11_SAMPLER_DESC>& samplerDescs,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(device, path, descriptions)
{
	_type = ShaderType::PS;

	if (FAILED(device->CreatePixelShader(ptr, size, NULL, &_psPtr)))
		__debugbreak();

	UINT numSamplers = samplerDescs.size();
	_samplers.resize(numSamplers);

	for (UINT i = 0; i < numSamplers; ++i)
		Sampler::SetUp(device, &samplerDescs[i], *_samplers[i].GetAddressOf());
}


void PixelShader::setBuffers(ID3D11DeviceContext* cont)
{
	for (int i = 0; i < _cbuffers.size(); ++i)
	{
		_cbuffers[i].bindToPS(cont, _cbufferMetaData[i]._slot);
	}

	// THIS CAN'T WORK UNLESS THEY ARE CONTIGUOUS IN MEMORY BUT IT'S FASTER! @TODO REORGANIZE THAT PART
	//if(_cbuffers.size() > 0)
		//cont->PSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]._cbPtr);
}