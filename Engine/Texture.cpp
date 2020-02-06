#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_PERLIN_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "stb_image_write.h"
#include "stb_perlin.h"
#include "WICTextureLoader.h"
#include "Texture.h"
#include "Perlin.h"


Texture::Texture() {}



Texture::Texture(ID3D11Device* device, const std::string& fileName) : _fileName(fileName)
{
	if (!LoadFromStoredPath())
	{
		OutputDebugStringA("Texture not in file, checking memory... \n");
		return;
	}

	SetUpAsResource(device);
}



Texture::Texture(const std::string& fileName) : _fileName(fileName)
{
	if (!LoadFromStoredPath())
	{
		OutputDebugStringA("Texture not in file, checking memory... \n");
	}
}



bool Texture::LoadFromStoredPath()
{
	try
	{
		_data = stbi_load(_fileName.c_str(), &w, &h, &n, 4);	//4?
		return (_data != nullptr);
	}
	catch (...)
	{
		OutputDebugStringA( ("Error loading texture '" + _fileName + "' \n").c_str() );
		return false;
	}
}



bool Texture::LoadFromFile(std::string path)
{
	_fileName = path;

	try
	{
		_data = stbi_load(path.c_str(), &w, &h, &n, 4);	//4?
		return (_data != nullptr);
	}
	catch (...)
	{
		OutputDebugStringA(("Error loading texture '" + _fileName + "' \n").c_str());
		return false;
	}
}



std::vector<float> Texture::LoadAsFloatVec(const std::string& path)
{
	float* temp;
	try
	{
		int tw, th, tn;

		//alas...//can't think of a way to avoid a copy if I want a vector, but this is rarely used so it's ok I guess
		temp = stbi_loadf(path.c_str(), &tw, &th, &tn, 0);
		std::vector<float> result(temp, temp + tw * th * tn);

		delete temp;

		return result;
	}
	catch (...)
	{
		if (temp)
			delete temp;

		OutputDebugStringA(("Error loading texture '" + path + "' \n").c_str());
		return std::vector<float>();
	}
}



bool Texture::LoadFromMemory(const unsigned char* data, size_t size)
{
	try
	{
		_data = stbi_load_from_memory(data, size, &w, &h, &n, 4);

		return (_data != nullptr);
	}
	catch (...)
	{
		OutputDebugStringA("Error loading texture from memory. \n");
		return false;
	}
}



bool Texture::LoadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin)
{
	w = perlin._w;
	h = perlin._h;
	_data = perlin.getUCharVector().data();

	return SetUpAsResource(device, DXGI_FORMAT::DXGI_FORMAT_R8_UNORM);
}



void Texture::LoadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path)
{
	std::wstring temp(path.begin(), path.end());
	const wchar_t* widecstr = temp.c_str();

	HRESULT result = DirectX::CreateWICTextureFromFile(device, context, widecstr, nullptr, &srv, 0);

	if (FAILED(result))
	{
		OutputDebugStringA("Can't create texture2d with mip levels (WIC). \n");
		exit(4201);
	}
}



bool Texture::SetUpAsResource(ID3D11Device* device, DXGI_FORMAT format) 
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = w;
	desc.Height = h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	//for now only supports this but I should make a map/table of sorts really... @TODO
	UINT pixelWidth = format == DXGI_FORMAT::DXGI_FORMAT_R8_UNORM ? 1 : 4;

	D3D11_SUBRESOURCE_DATA texData;
	texData.pSysMem = (void *)_data;
	texData.SysMemPitch = desc.Width * pixelWidth;
	texData.SysMemSlicePitch = 0;

	if (FAILED(device->CreateTexture2D(&desc, &texData, &texId)))
	{
		OutputDebugStringA("Can't create texture2d. \n");
		exit(42);
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = desc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	if (FAILED(device->CreateShaderResourceView(texId, &shaderResourceViewDesc, &srv)))
	{
		OutputDebugStringA("Can't create shader resource view. \n");
		exit(43);
	}

	//delete data;

	return true;
}



//for comp: 1=Y, 2=YA, 3=RGB, 4=RGBA 
void Texture::WriteToFile(const std::string& targetFile, int w, int h, int comp, void* data, int stride_in_bytes)
{
	try
	{
		int result = stbi_write_png(targetFile.c_str(), w, h, comp, data, stride_in_bytes);
	}
	catch (...)
	{
		OutputDebugStringA( ("Error writing texture to '" + targetFile + "'; ").c_str() );
		return;
	}
}



float Texture::Perlin3D(float x, float  y, float z, UINT xw, UINT yw, UINT zw)
{
	return stb_perlin_noise3(x, y, z, xw, yw, zw);
}



float Texture::Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw, UINT yw, UINT zw)
{
	return stb_perlin_fbm_noise3(x, y, z, lacunarity, gain, octaves, xw, yw, zw);
}



float Texture::Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw, UINT yw, UINT zw)
{
	return stb_perlin_turbulence_noise3(x, y, z, lacunarity, gain, octaves, xw, yw, zw);
}



float Texture::Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw, UINT yw, UINT zw)
{
	return stb_perlin_ridge_noise3(x, y, z, lacunarity, gain, offset, octaves, xw, yw, zw);
}



std::vector<float> Texture::generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves, UINT xw, UINT yw, UINT zw)
{
	//std::vector<unsigned char> result;
	std::vector<float> result;
	
	result.reserve(w * h);

	float wInverse = 1.f / (float)w;
	float hInverse = 1.f / (float)h;

	for (int i = 0; i < w; ++i)
	{
		for (int j = 0; j < h; ++j)
		{
			float x = (float)i * wInverse;
			float y = (float)j * hInverse;

			float noiseVal = Texture::Turbulence3D(x, y, z, lacunarity, gain, octaves, xw, yw, zw);
			result.push_back(noiseVal);

			//unsigned char uc = ((rgb + 1.f) * 0.5f) * 255;
			//curData.push_back(uc);
		}
	}

	return result;
}



std::vector<float> Texture::generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw, UINT yw, UINT zw)
{
	//std::vector<unsigned char> result;
	std::vector<float> result;
	
	//result.reserve(w * h);
	result.reserve(w * h);

	float wInverse = 1.f / (float)w;
	float hInverse = 1.f / (float)h;

	for (int i = 0; i < w; ++i)
	{
		for (int j = 0; j < h; ++j)
		{
			float x = (float)i * wInverse;
			float y = (float)j * hInverse;

			float noiseVal = Texture::Ridge3D(x, y, z, lacunarity, gain, offset, octaves, xw, yw, zw);
			result.push_back(noiseVal);

			//unsigned char uc = ((rgb + 1.f) * 0.5f) * 255;	works only if -1, 1 which is not the case
			//curData.push_back(uc);
		}
	}

	return result;
}