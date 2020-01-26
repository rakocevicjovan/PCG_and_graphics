#pragma once
#include "ShaderCompiler.h"
#include "Math.h"
#include "Light.h"
#include "CBuffer.h"
#include "TextureRole.h"


enum class SHADER_TYPE { VS, GS, PS, TS, HS };

struct TexLayout
{
	uint8_t _index;
	uint8_t _count;
};



class Shader
{
protected:
	static UINT ID_COUNTER;
	Shader(const ShaderCompiler& shc, const std::wstring& path, const std::vector<D3D11_BUFFER_DESC>& descriptions);

public:
	const UINT _id;
	SHADER_TYPE _type;
	std::wstring _path;

	std::vector<CBufferMeta> _bufferMetaData;
	std::vector<ID3D11Buffer*> _cbuffers;

	//contains texture and constant buffer meta data along with register indices for each of them
	ShRef::ShaderMetadata _shMetaData;

	// _textureRegisters[TextureRole] contains the first binding slot of that texture type and number of them required by the shader
	// which does mean that they have to be in contiguous texture registers but this is not a problem
	TexLayout _textureRegisters[TextureRole::NUM_ROLES];

	// alternative to the automatic system for custom data
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