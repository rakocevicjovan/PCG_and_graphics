#pragma once
#include "Math.h"
#include <vector>
#include <d3d11.h>
#include "Texture.h"
#include "Light.h"


//Shader structs for drawing
struct SIBase 
{};


struct MatrixBuffer : public SIBase
{
	SMatrix world;
	SMatrix view;
	SMatrix projection;
};

struct ShadowMatrixBuffer : public MatrixBuffer
{
	SMatrix lightView, lightProjection;
};


struct VariableBuffer : public SIBase
{
	float deltaTime;
	SVec3 padding;
};


struct LightBuffer : public SIBase
{
		SVec3 alc;
		float ali;
		SVec3  dlc;
		float dli;
		SVec3 slc;
		float sli;
		SVec4 pos;
		SVec4 ePos;
};

struct LightBufferType2 : public SIBase
{
	SVec3 lightPosition;
	float padding;
};













//Shader parameters for frame by frame drawing
struct SPBase
{};

struct DrawParams : public SPBase
{
	SMatrix m, v, p;
	
	float deltaTime;

	PointLight pLight;
	SVec3 camPos;
};


class Model;
class PointLight;

struct ShaderParametersLight : public SPBase
{
	ID3D11DeviceContext* deviceContext;
	Model* model;
	SMatrix* view;
	SMatrix* proj;
	PointLight* dLight;
	SVec3* eyePos;
	float deltaTime;
};






//shader structs for initialization
struct InitParamsBase 
{};

struct InitParamsLight : public InitParamsBase
{
	static const unsigned int layoutSize = 3;
	static D3D11_INPUT_ELEMENT_DESC polygonLayout[layoutSize];

	InitParamsLight()
	{
		polygonLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		polygonLayout[1] = { "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		polygonLayout[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	}

	InitParamsLight* get()
	{
		return this;
	}
};

struct BufferDescsLight
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC variableBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;

	BufferDescsLight()
	{
		matrixBufferDesc = { sizeof(MatrixBuffer), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
		variableBufferDesc = { sizeof(VariableBuffer), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
		lightBufferDesc = { sizeof(LightBuffer), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
	}

	BufferDescsLight* get()
	{
		return this;
	}
};

struct SamplerDescLight
{
	D3D11_SAMPLER_DESC samplerDesc;
	SamplerDescLight()
	{
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	}
	
	SamplerDescLight* get()
	{
		return this;
	}
};