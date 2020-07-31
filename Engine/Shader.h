#pragma once
#include "ShaderCompiler.h"
#include "Math.h"
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
	Shader(const ShaderCompiler& shc, const std::wstring& path, const std::vector<D3D11_BUFFER_DESC>& descriptions);

public:
	UINT _id;
	SHADER_TYPE _type;
	std::wstring _path;

	//contains texture and constant buffer meta data along with register indices for each of them, reflected from shader directly
	ShRef::SRShaderMetadata _refShMetaData;

	//set manually, similar to reflected data minus the texture bind slots
	std::vector<CBuffer> _cbuffers;

	// _textureRegisters[TextureRole] contains the first binding slot of that texture type and number of them required by the shader
	TexLayout _textureRegisters[TextureRole::NUM_ROLES];

	// alternative to the automatic system, for custom data
	bool updateCBufferDirectly(ID3D11DeviceContext* cont, void* data, uint8_t index);

	inline void describeBuffers(const std::vector<CBufferMeta>& meta)
	{
		for (int i = 0; i < _cbuffers.size(); i++)
		{
			_cbuffers[i]._metaData = meta[i];
		}
	}
};



class VertexShader : public Shader
{
public:
	ID3D11VertexShader* _vsPtr;
	ID3D11InputLayout* _layout;

	VertexShader(
		const ShaderCompiler& shc, 
		const std::wstring& path, 
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc, 
		const std::vector<D3D11_BUFFER_DESC>& descriptions);

	~VertexShader()
	{
		if (_vsPtr)
		{
			_vsPtr->Release();
		}
	}

	void setBuffers(ID3D11DeviceContext* cont);


	// Separate function for vs and ps to reduce the number of potential semantics to check for... slight gain but hopefully worth
	template <typename RenderItem>
	void updateBuffersAuto(ID3D11DeviceContext* cont, const RenderItem& ri) const
	{
		D3D11_MAPPED_SUBRESOURCE mr;

		for (int i = 0; i < _cbuffers.size(); i++)
		{
			ID3D11Buffer* curBuffer = _cbuffers[i]._cbPtr;
			CBuffer::map(cont, curBuffer, mr);

			for (const CBufferFieldDesc& cbfd : _cbuffers[i]._metaData._fields)
			{
				if (cbfd._content == CBUFFER_FIELD_CONTENT::TRANSFORM)
				{
					CBuffer::updateField(curBuffer, &(ri._transform.Transpose()), cbfd._size, cbfd._offset, mr);
				}
			}

			CBuffer::unmap(cont, curBuffer);
		}
	}
};



class PixelShader : public Shader
{
public:
	ID3D11PixelShader* _psPtr;
	ID3D11SamplerState* _sState;

	PixelShader(
		const ShaderCompiler& shc,
		const std::wstring& path,
		const D3D11_SAMPLER_DESC& samplerDesc,
		const std::vector<D3D11_BUFFER_DESC>& descriptions);

	~PixelShader()
	{
		if (_psPtr)
		{
			_psPtr->Release();
		}
	}

	void setBuffers(ID3D11DeviceContext* cont);



	template <typename RenderItem>
	void updateBuffersAuto(ID3D11DeviceContext* cont, const RenderItem& ri) const
	{
		// None defined so far
		/*
		D3D11_MAPPED_SUBRESOURCE mr;

		for (int i = 0; i <_cbuffers.size(); i++)
		{
			ID3D11Buffer* curBuffer = _cbuffers[i]._cbPtr;
			CBuffer::map(cont, curBuffer, mr);

			for (const CBufferFieldDesc& cbfd : _cbuffers[i]._metaData._fields) {}

			CBuffer::unmap(cont, curBuffer);
		}
		*/
	}
};