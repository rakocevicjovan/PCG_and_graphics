#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STBI_MSC_SECURE_CRT
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "WICTextureLoader.h"
#include "Texture.h"
#include "Perlin.h"


Texture::Texture() : _dxID(nullptr), _arraySrv(nullptr) {}



Texture::Texture(ID3D11Device* device, const std::string& fileName) : _fileName(fileName), _dxID(nullptr), _arraySrv(nullptr)
{
	if (!loadFromStoredPath())
	{
		OutputDebugStringA("Texture not in file, checking memory... \n");
		return;
	}

	setUpAsResource(device);
}



Texture::Texture(const std::string& fileName) : _fileName(fileName), _dxID(nullptr), _arraySrv(nullptr)
{
	if (!loadFromStoredPath())
	{
		OutputDebugStringA("Texture not in file, checking memory... \n");
	}
}



int Texture::GetFormatFromFile(const char* filename)
{
	int w, h, n;
	stbi_info(filename, &w, &h, &n);	//_nc is our format but holds this temporarily
	return n == 3 ? 4 : n;
}



int Texture::GetFormatFromMemory(const unsigned char* data, size_t size)
{
	int w, h, n;
	stbi_info_from_memory(data, size, &w, &h, &n);
	return n == 3 ? 4 : n;
}



void Texture::loadFromFile(const char* filename)
{
	int fileFormat, desiredFormat, w, h;
	desiredFormat = GetFormatFromFile(filename);
	_mdata = std::shared_ptr<unsigned char[]>(stbi_load(filename, &w, &h, &fileFormat, desiredFormat));
	_w = w;
	_h = h;
	_nc = desiredFormat;
	_snc = fileFormat;
}



bool Texture::loadFromStoredPath()
{
	try
	{
		loadFromFile(_fileName.c_str());
		return (_mdata.get() != nullptr);
	}
	catch (...)
	{
		OutputDebugStringA( ("Error loading texture '" + _fileName + "' \n").c_str() );
		return false;
	}
}



bool Texture::loadFromPath(const char* path)
{
	_fileName = path;

	try
	{
		loadFromFile(path);
		return (_mdata.get() != nullptr);
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

		// Staying as it is to avoid reworking strife level but should remove the copy.
		temp = stbi_loadf(path.c_str(), &tw, &th, &tn, 4);
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



bool Texture::loadFromMemory(const unsigned char* data, size_t size)
{
	try
	{
		int fileFormat, desiredFormat, w, h;
		desiredFormat = GetFormatFromMemory(data, size);
		unsigned char* wat = stbi_load_from_memory(data, size, &w, &h, &fileFormat, desiredFormat);
		_mdata = std::shared_ptr<unsigned char[]>(wat);
		_w = w;
		_h = h;
		_nc = desiredFormat;
		_snc = fileFormat;

		return (_mdata.get() != nullptr);
	}
	catch (...)
	{
		OutputDebugStringA("Error loading texture from memory. \n");
		return false;
	}
}



bool Texture::loadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin)
{
	_w = perlin._w;
	_h = perlin._h;
	_nc = 1;
	
	_mdata = std::shared_ptr<unsigned char[]>(perlin.getUCharVector().data());

	return setUpAsResource(device);
}



bool Texture::loadRegion()
{
	
	return false;
}



bool Texture::loadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path)
{
	std::wstring temp(path.begin(), path.end());
	const wchar_t* widecstr = temp.c_str();

	HRESULT result = DirectX::CreateWICTextureFromFile(device, context, widecstr, nullptr, &_arraySrv, 0);

	if (FAILED(result))
	{
		OutputDebugStringA("Can't create texture2d with mip levels (WIC). \n");
		exit(4201);
		return false;
	}
	return true;
}



bool Texture::setUpAsResource(ID3D11Device* device, bool deleteData)
{
	DXGI_FORMAT inferredFormat = N_TO_FORMAT_DX11[_nc - 1];

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = _w;
	desc.Height = _h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = inferredFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	// For now always uses 1 byte per channel textures, @todo add byte width flag as well
	UINT pixelWidth = _nc;

	D3D11_SUBRESOURCE_DATA texData;
	texData.pSysMem = (void *)(_mdata.get());
	texData.SysMemPitch = desc.Width * pixelWidth;
	texData.SysMemSlicePitch = 0;

	createGPUResource(device, &desc, &texData);


	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = desc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	if (FAILED(device->CreateShaderResourceView(_dxID.Get(), &shaderResourceViewDesc, &_arraySrv)))
	{
		OutputDebugStringA("Can't create shader resource view. \n");
		exit(43);
	}

	if (deleteData)
		freeMemory();

	return true;
}



bool Texture::createGPUResource(ID3D11Device* device, D3D11_TEXTURE2D_DESC* desc, D3D11_SUBRESOURCE_DATA* data)
{
	if (FAILED(device->CreateTexture2D(desc, data, &_dxID)))
	{
		OutputDebugStringA("Can't create texture2d. \n");
		exit(42);
	}

	return true;
}



//for comp: 1=Y, 2=YA, 3=RGB, 4=RGBA 
void Texture::SaveAsPng(const char* targetFile, int w, int h, int comp, const void* data, int stride_in_bytes)
{
	try
	{
		int result = stbi_write_png(targetFile, w, h, comp, data, stride_in_bytes);
	}
	catch (...)
	{
		std::string errorMessage = "Error writing texture to file: ";
		errorMessage += targetFile;
		OutputDebugStringA(errorMessage.c_str());
	}
}