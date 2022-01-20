#pragma once
#include <string>
#include <d3d11_4.h>
#include "DDSTextureLoader.h"

class CubeMap
{
	ID3D11Texture2D* texPtr;

public:

	static inline D3D11_TEXTURE2D_DESC CreateCubeMapDesc(UINT edgeLength, bool renderTarget, DXGI_FORMAT format)
	{
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = edgeLength;
		texDesc.Height = edgeLength;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 6;
		texDesc.Format = format;
		texDesc.CPUAccessFlags = 0;
		texDesc.SampleDesc = { 1, 0 };
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		if (renderTarget)
			texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;

		return texDesc;
	}


	static void CreateCubeMap(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& texDesc, ID3D11Texture2D** texPtr)
	{
		if (FAILED(device->CreateTexture2D(&texDesc, 0, texPtr)))
		{
			OutputDebugStringA("Failed to create cube map texture. \n");
			exit(520);
		}
	}


	static void CreateCubeMapSrv(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& texDesc, 
		ID3D11Texture2D* texPtr, ID3D11ShaderResourceView** shResView)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
		srvd.Format = texDesc.Format;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvd.TextureCube.MipLevels = texDesc.MipLevels;
		srvd.TextureCube.MostDetailedMip = 0;

		if (FAILED(device->CreateShaderResourceView(texPtr, &srvd, shResView)))
		{
			OutputDebugStringA("Failed to create shader resource view. \n");
			exit(521);
		}
	}


	// Utility function for skybox rendering. Loads the maps once, no render targets / depth stencil textures involved
	static void LoadCubeMapFromFile(ID3D11Device* device, std::string_view filename, UINT edgeLength, ID3D11Texture2D** texPtr, ID3D11ShaderResourceView** shResView)
	{
		D3D11_TEXTURE2D_DESC texDesc = CreateCubeMapDesc(edgeLength, false, DXGI_FORMAT_R32G32B32A32_FLOAT);

		CreateCubeMap(device, texDesc, texPtr);

		CreateCubeMapSrv(device, texDesc, *texPtr, shResView);

		std::wstring widestr = std::wstring(filename.begin(), filename.end());

		if (FAILED(DirectX::CreateDDSTextureFromFileEx(device, widestr.c_str(), 0u, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
			0u, D3D11_RESOURCE_MISC_TEXTURECUBE, false, (ID3D11Resource**)texPtr, shResView, (DirectX::DDS_ALPHA_MODE*)nullptr)))
		{
			OutputDebugStringA("Failed to load dds texture \n");
			exit(522);
		}
	}
};