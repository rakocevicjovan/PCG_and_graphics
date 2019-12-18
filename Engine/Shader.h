#pragma once

#include "ShaderCompiler.h"
#include "Math.h"
#include "Light.h"
#include "CBuffer.h"


enum class SHADER_TYPE { VS, GS, PS };


class Shader
{
protected:
	static UINT ID_COUNTER;

public:
	const UINT _id;
	SHADER_TYPE type;
	std::string path;

	std::vector<CBufferMeta> _bufferMetaData;
	std::vector<ID3D11Buffer*> _cbuffers;

	Shader(const ShaderCompiler& shc, const std::wstring& path, const std::vector<D3D11_BUFFER_DESC>& descriptions);

	//copout for when the automatic system doesn't work
	bool updateCBufferDirectly(ID3D11DeviceContext* cont, void* data, uint8_t index);

	inline void describeBuffers(const std::vector<CBufferMeta>& meta)
	{
		_bufferMetaData = meta;
	}
};



class VertexShader : public Shader
{
public:
	ID3D11VertexShader* _vShader;
	ID3D11InputLayout* _layout;

	VertexShader(
		const ShaderCompiler& shc, 
		const std::wstring& path, 
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc, 
		const std::vector<D3D11_BUFFER_DESC>& descriptions);

	void setBuffers(ID3D11DeviceContext* cont);
};



class PixelShader : public Shader
{
public:
	ID3D11PixelShader* _pShader;
	ID3D11SamplerState* _sState;

	PixelShader(
		const ShaderCompiler& shc,
		const std::wstring& path,
		const D3D11_SAMPLER_DESC& samplerDesc,
		const std::vector<D3D11_BUFFER_DESC>& descriptions);

	void setBuffers(ID3D11DeviceContext* cont);
};