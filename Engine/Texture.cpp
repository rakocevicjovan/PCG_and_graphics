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
#include "Texture.h"
#include "Perlin.h"


Texture::Texture() {}



Texture::Texture(ID3D11Device* device, const std::string& fileName) {

	this->fileName = fileName;
		
	if (!Load()) {
		OutputDebugStringA("Texture not in file, checking memory... \n");
		return;
	}

	Setup(device);
}



bool Texture::Load() {

	try {
		data = stbi_load(fileName.c_str(), &w, &h, &n, 4);	//4?

		return (data != nullptr);
	}
	catch (...) {
		OutputDebugStringA( ("Error loading texture '" + fileName + "' \n").c_str() );
		return false;
	}
}



bool Texture::LoadFromFile(std::string path) {

	fileName = path;

	try {
		data = stbi_load(path.c_str(), &w, &h, &n, 4);	//4?
		return (data != nullptr);
	}
	catch (...) {
		OutputDebugStringA(("Error loading texture '" + fileName + "' \n").c_str());
		return false;
	}
}

//has to be set up if used like this!!
bool Texture::LoadFromMemory(const aiTexture *texture, ID3D11Device* device) {

	try {
		if (texture->mHeight == 0)
			data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth, &w, &h, &n, 4);
		else
			data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth * texture->mHeight, &w, &h, &n, 4);

		Setup(device);

		return (data != nullptr);
	}
	catch (...) {
		OutputDebugStringA("Error loading texture from memory. \n");
		return false;
	}
}



bool Texture::LoadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin)
{
	w = perlin._w;
	h = perlin._h;
	data = perlin.getUCharVector().data();

	return Setup(device, true);
}



bool Texture::Setup(ID3D11Device* device, bool grayscale) 
{
	desc.Width = w;
	desc.Height = h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = grayscale ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	texData.pSysMem = (void *)data;
	texData.SysMemPitch = grayscale ? desc.Width : desc.Width * 4;
	texData.SysMemSlicePitch = 0;

	if (FAILED(device->CreateTexture2D(&desc, &texData, &texId)))
	{
		OutputDebugStringA("Can't create texture2d. \n");
		exit(42);
	}


	shaderResourceViewDesc.Format = desc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	if (FAILED(device->CreateShaderResourceView(texId, &shaderResourceViewDesc, &srv)))
	{
		OutputDebugStringA("Can't create shader resource view. \n");
		exit(43);
	}

	return true;
}



//for comp: 1=Y, 2=YA, 3=RGB, 4=RGBA 
void Texture::WriteToFile(const std::string& targetFile, int w, int h, int comp, void* data, int stride_in_bytes) {

	try {
		int result = stbi_write_png(targetFile.c_str(), w, h, comp, data, stride_in_bytes);
	}
	catch (...) {
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
	std::vector<unsigned char> curData;
	std::vector<float> result;
	
	curData.reserve(w * h);
	result.reserve(w * h);

	float wInverse = 1.f / (float)w;
	float hInverse = 1.f / (float)h;

	for (int i = 0; i < w; ++i)
	{
		for (int j = 0; j < h; ++j)
		{
			float x = (float)i * wInverse;
			float y = (float)j * hInverse;

			float rgb = Texture::Turbulence3D(x, y, z, lacunarity, gain, octaves, xw, yw, zw);
			int r = (int)((rgb + 1.f) * 0.5f * 255.f);
			unsigned char uc = (unsigned char)r;

			result.push_back(rgb);
			curData.push_back(uc);
		}
	}

	return result;
}



std::vector<float> Texture::generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw, UINT yw, UINT zw)
{
	std::vector<unsigned char> curData;
	std::vector<float> result;
	
	curData.reserve(w * h);
	result.reserve(w * h);

	float wInverse = 1.f / (float)w;
	float hInverse = 1.f / (float)h;

	for (int i = 0; i < w; ++i)
	{
		for (int j = 0; j < h; ++j)
		{
			float x = (float)i * wInverse;
			float y = (float)j * hInverse;

			float rgb = Texture::Ridge3D(x, y, z, lacunarity, gain, offset, octaves, xw, yw, zw);
			int r = (int)((rgb + 1.f) * 0.5f);
			unsigned char uc = (unsigned char)r;

			result.push_back(rgb);
			curData.push_back(uc);
		}
	}

	return result;
}



void Texture::LoadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path)
{
	std::wstring temp(path.begin(), path.end());
	const wchar_t* widecstr = temp.c_str();

	/*ID3D11Resource* resource;

	HRESULT hr = resource->QueryInterface(IID_ID3D11Texture2D, (void **)&texId);

	if (FAILED(hr))
	{
		OutputDebugStringA("Ma jebem mu ja sve zivo i mrtvo... \n");
		exit(4202);
	}*/

	HRESULT result = DirectX::CreateWICTextureFromFile(device, context, widecstr, nullptr, &srv, 0);

	if (FAILED(result))
	{
		OutputDebugStringA("Can't create texture2d with mip levels (WIC). \n");
		exit(4201);
	}
}