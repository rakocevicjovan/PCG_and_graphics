#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include "ShaderCompiler.h"
#include "Math.h"
#include "Light.h"
#include "CBuffer.h"


enum class SHADER_TYPE { VS, GS, PS };


class Shader
{
	static UINT ID_COUNTER;

public:
	UINT _id;
	SHADER_TYPE type;
	std::string path;

	std::vector<CBufferMeta> descs;
	std::vector<ID3D11Buffer*> _cbuffers;



	Shader()
	{
		_id = ID_COUNTER++;
	}



	Shader(ID3D11Device* dev, std::string path, std::vector<D3D11_BUFFER_DESC>& descriptions)
	{
		//this feels horrible but whatever... will do it better once it works I guess
		for (int i = 0; i < descriptions.size(); ++i)
		{
			_cbuffers.push_back(nullptr);
			if (FAILED(dev->CreateBuffer(&descriptions[i], NULL, &_cbuffers.back())))
			{
				exit(599);	
			}
		}
		_id = ID_COUNTER++;
	}


	//copout for when the automatic system doesn't work
	bool updateCBufferDirectly(ID3D11DeviceContext* cont, void* data, uint8_t index)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		if (FAILED(cont->Map(_cbuffers[index], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			return false;

		void* dataPtr = mappedResource.pData;
		memcpy(dataPtr, data, descs[index]._size);
		cont->Unmap(_cbuffers[index], 0);

		return true;
	}

};



class VertexShader : public Shader
{
public:
	ID3D11VertexShader* _vShader;
	ID3D11InputLayout* _layout;

	void setBuffers(ID3D11DeviceContext* cont)
	{
		for (auto& buffer : _cbuffers)
		{
			cont->VSSetConstantBuffers(0, 1, &buffer);
		}
		
	}
};



class PixelShader : public Shader
{
public:
	ID3D11PixelShader* _pShader;
	ID3D11SamplerState* _sState;

	/*
	bool populateBuffers(ID3D11DeviceContext* cont, PointLight pLight, SVec3 ePos)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		LightBuffer* dataPtr;

		if (FAILED(cont->Map(_cbuffers[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			return false;

		dataPtr = (LightBuffer*)mappedResource.pData;
		dataPtr->alc = pLight.alc;
		dataPtr->ali = pLight.ali;
		dataPtr->dlc = pLight.dlc;
		dataPtr->dli = pLight.dli;
		dataPtr->slc = pLight.slc;
		dataPtr->sli = pLight.sli;
		dataPtr->pos = pLight.pos;
		dataPtr->ePos = Math::fromVec3(ePos, 1.f);
		cont->Unmap(_cbuffers[0], 0);
		cont->PSSetConstantBuffers(0, 1, &_cbuffers[0]);

		return true;
	}
	*/
};