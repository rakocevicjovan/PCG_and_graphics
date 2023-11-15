#pragma once


class VirtualTexture
{
	//@TODO use this when everything else works
	//ATL::CComPtr<ID3D11Texture2D> _texPtr;
	//ATL::CComPtr<ID3D11Buffer> _tilePoolBuffer;

	ID3D11Texture2D* _texPtr;
	ID3D11Buffer* _tilePoolBuffer;

	void createTilePool(ID3D11Device* device, UINT byteCapacity, UINT byteStride)
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = byteCapacity;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TILE_POOL;
		desc.StructureByteStride = 0u;
		desc.Usage = D3D11_USAGE_DEFAULT;	//dynamic, staging and immutable not suported

		device->CreateBuffer(&desc, nullptr, &_tilePoolBuffer);
	}



	void createGPUResource(ID3D11Device* device)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 0u;
		desc.BindFlags = D3D11_RESOURCE_MISC_TILED;	// Not here probably... 
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		
		device->CreateTexture2D(&desc, nullptr, &_texPtr);
	}


};


/*
	// Some useful stuff from msdn

	Supported Bind Flags
	D3D11_BIND_SHADER_RESOURCE, _RENDER_TARGET, _DEPTH_STENCIL, or _UNORDERED_ACCESS.

	NOT supported:
	_CONSTANT_BUFFER, _VERTEX_BUFFER [note that binding a tiled Buffer as an SRV/UAV/RTV is still ok],
	_INDEX_BUFFER, _STREAM_OUTPUT, _BIND_DECODER, or _BIND_VIDEO_ENCODER.

	To be able to save memory, an application has to not only shrink a tile pool
	but also remove/remap existing mappings past the end of the new smaller tile pool size.
*/