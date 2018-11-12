#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "stb_image_write.h"
#include "Texture.h"



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


bool Texture::Setup(ID3D11Device* device) {
	
	HRESULT res;

	desc.Width = w;
	desc.Height = h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	//if(n == 4){}
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//DXGI_FORMAT_R8G8B8A8_SINT	DXGI_FORMAT_R8G8B8A8_UNORM
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	texData.pSysMem = (void *)data;
	texData.SysMemPitch = desc.Width * 4; //* sizeof(float);
	texData.SysMemSlicePitch = 0;

	res = device->CreateTexture2D(&desc, &texData, &texId);
	if (FAILED(res)) {
		OutputDebugStringA("Can't create texture2d. \n");
		exit(42);
	}


	shaderResourceViewDesc.Format = desc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	res = device->CreateShaderResourceView(texId, &shaderResourceViewDesc, &srv);	//&resViewDesc
	if (FAILED(res)) {
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
