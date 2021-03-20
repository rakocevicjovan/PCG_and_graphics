//#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "d3dcompiler.lib")

#include "Shader.h"
#include "Sampler.h"
#include "CBuffer.h"
#include <d3d11_4.h>



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
	_cbuffers[index].update(cont, data, _cbuffers[index]._metaData._size);
}



VertexShader::VertexShader(
	const ShaderCompiler& shc,
	const std::wstring& path,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc,
	const std::vector<D3D11_BUFFER_DESC>& descriptions)
	: Shader(shc.getDevice(), path, descriptions)
{
	_type = SHADER_TYPE::VS;

	if (!shc.compileVS(path, inputLayoutDesc, _vsPtr, _layout))
		__debugbreak();
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
	_type = SHADER_TYPE::VS;
	if (FAILED(device->CreateVertexShader(ptr, size, NULL, &_vsPtr)))
		__debugbreak();

	if (FAILED(device->CreateInputLayout(inLay.data(), inLay.size(), ptr, size, &_layout)))
		__debugbreak();
}



VertexShader::~VertexShader()
{
	if (_vsPtr)
		_vsPtr->Release();
	if (_layout)
		_layout->Release();
}

VertexShader::VertexShader(const VertexShader& other)
	: Shader(other), _vsPtr(other._vsPtr), _layout(other._layout)
{
	if (_vsPtr)
		_vsPtr->AddRef();
	if (_layout)
		_layout->AddRef();
}

VertexShader::VertexShader(const VertexShader&& other)
	: Shader(other), _vsPtr(other._vsPtr), _layout(other._layout)
{
	if (_vsPtr)
		_vsPtr->AddRef();
	if (_layout)
		_layout->AddRef();
}


VertexShader& VertexShader::operator= (VertexShader other)
{
	_vsPtr = other._vsPtr;
	if (_vsPtr)
		_vsPtr->AddRef();

	_layout = other._layout;
	if (_layout)
		_layout->AddRef();

	_id = other._id;
	_type = other._type;
	_path = other._path;

	_refShMetaData = other._refShMetaData;
	_cbuffers = other._cbuffers;

	_textureRegisters = other._textureRegisters;

	return *this;
}



void VertexShader::setBuffers(ID3D11DeviceContext* cont)
{
	for (int i = 0; i < _cbuffers.size(); ++i)
	{
		_cbuffers[i].bindToVS(cont, (_cbuffers[i]._metaData._slot));
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
	_type = SHADER_TYPE::PS;
	shc.compilePS(path, _psPtr, &_refShMetaData);
	
	UINT numSamplers = samplerDescs.size();
	_samplers.resize(numSamplers);

	for (UINT i = 0; i < numSamplers; ++i)
		Sampler::setUp(shc.getDevice(), &samplerDescs[i], _samplers[i]);
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
	_type = SHADER_TYPE::PS;

	if (FAILED(device->CreatePixelShader(ptr, size, NULL, &_psPtr)))
		__debugbreak();

	UINT numSamplers = samplerDescs.size();
	_samplers.resize(numSamplers);

	for (UINT i = 0; i < numSamplers; ++i)
		Sampler::setUp(device, &samplerDescs[i], _samplers[i]);
}



void PixelShader::setBuffers(ID3D11DeviceContext* cont)
{
	for (int i = 0; i < _cbuffers.size(); ++i)
	{
		_cbuffers[i].bindToPS(cont, _cbuffers[i]._metaData._slot);
	}

	// THIS CAN'T WORK UNLESS THEY ARE CONTIGUOUS IN MEMORY BUT IT'S FASTER! @TODO REORGANIZE THAT PART
	//if(_cbuffers.size() > 0)
		//cont->PSSetConstantBuffers(0, _cbuffers.size(), &_cbuffers[0]._cbPtr);
}