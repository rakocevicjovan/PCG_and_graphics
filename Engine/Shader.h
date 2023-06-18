#pragma once
#include "ShaderCompiler.h"
#include "Math.h"
#include "TextureRole.h"
#include "CBuffer.h"

enum class ShaderType : uint8_t { VS, GS, PS, TS, HS, SHADER_TYPE_COUNT };

struct TexLayout
{
	uint8_t _index;
	uint8_t _count;
};

class Shader
{
protected:
	Shader() = default;
	Shader(ID3D11Device* device, const std::wstring& path, const std::vector<D3D11_BUFFER_DESC>& descriptions);

public:
	//uint64_t _id{};
	ShaderType _type{};
	std::wstring _path{};

	//contains texture and constant buffer meta data along with register indices for each of them, reflected from shader directly
	ShRef::SRShaderMetadata _refShMetaData;

	// Set manually for better control on what to keep and how it's stored, but otherwise identical to the shader reflection
	std::vector<CBuffer> _cbuffers;
	std::vector<CBufferMeta> _cbufferMetaData;
	// _textureRegisters[TextureRole] contains the first binding slot of that texture type and number of them required by the shader
	std::array<TexLayout, TextureRole::NUM_ROLES> _textureRegisters{};

	void updateCBufferDirectly(ID3D11DeviceContext* cont, void* data, uint8_t index);

	inline void describeBuffers(const std::vector<CBufferMeta>& meta)
	{
		_cbufferMetaData = meta;
	}
};



class VertexShader : public Shader
{
public:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> _vsPtr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> _layout;

	VertexShader() = default;

	VertexShader(
		const ShaderCompiler& shc, 
		const std::wstring& path, 
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc, 
		const std::vector<D3D11_BUFFER_DESC>& descriptions = {});

	VertexShader(
		ID3D11Device* device,
		void* ptr,
		UINT size,
		const std::wstring& path,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inLay,
		const std::vector<D3D11_BUFFER_DESC>& descriptions);


	void setBuffers(ID3D11DeviceContext* cont);


	inline void bind(ID3D11DeviceContext* context)
	{
		context->VSSetShader(_vsPtr.Get(), nullptr, 0);
		context->IASetInputLayout(_layout.Get());
	}
};



class PixelShader : public Shader
{
public:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> _psPtr;
	std::vector<Microsoft::WRL::ComPtr<ID3D11SamplerState>> _samplers;

	PixelShader() = default;

	PixelShader(
		const ShaderCompiler& shc,
		const std::wstring& path,
		const std::vector<D3D11_SAMPLER_DESC>& samplerDescs,
		const std::vector<D3D11_BUFFER_DESC>& descriptions = {});

	PixelShader(
		ID3D11Device* device,
		void* ptr,
		UINT size,
		const std::wstring& path,
		const std::vector<D3D11_SAMPLER_DESC>& samplerDescs,
		const std::vector<D3D11_BUFFER_DESC>& descriptions
	);

	void setBuffers(ID3D11DeviceContext* cont);

	inline void bind(ID3D11DeviceContext* context)
	{
		context->PSSetShader(_psPtr.Get(), nullptr, 0);

		for (UINT i = 0; i < _samplers.size(); ++i)
			context->PSSetSamplers(i, 1, _samplers[i].GetAddressOf());
	}
};