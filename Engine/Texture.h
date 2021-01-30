#pragma once
#include <string>
#include <vector>
#include <memory>
#include <d3d11_4.h>
#include <dxgiformat.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

#include "Resource.h"
#include "TextureRole.h"


namespace Procedural
{
	class Perlin;
	class TextureGen;
}

inline const std::vector<DXGI_FORMAT> N_TO_FORMAT_DX11
{
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R8G8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM,	// Thanks once again Microsoft
	DXGI_FORMAT_R8G8B8A8_UNORM
};


class Texture : public Resource
{
private:

	friend class Procedural::TextureGen;
	friend class cereal::access;
	
protected:
	
	uint16_t _w, _h;	// width, height, max 65536 maximum each
	uint8_t _nc, _snc;	// num channels in loaded data and storage data
	std::shared_ptr<unsigned char[]> _mdata;

	static int GetFormatFromFile(const char* filename);

	static int GetFormatFromMemory(const unsigned char* data, size_t size);

	void loadFromFile(const char* filename);

public:

	ID3D11Texture2D* _dxID;
	ID3D11ShaderResourceView* _srv;

	std::string _fileName;	// Helpful to debug, likely to be removed in release...

	Texture();
	Texture(ID3D11Device* device, const std::string& fileName);
	Texture(const std::string& fileName);
	Texture(const Texture& other);
	Texture(Texture&& other);
	Texture& Texture::operator=(const Texture& rhs);
	~Texture();

	bool loadFromStoredPath();
	bool loadFromPath(const char* path);
	bool loadFromMemory(const unsigned char* data, size_t size);
	bool loadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path);
	bool loadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin);
	bool loadRegion();

	bool setUpAsResource(ID3D11Device* device, bool deleteData = true);
	bool createGPUResource(ID3D11Device* device, D3D11_TEXTURE2D_DESC* desc, D3D11_SUBRESOURCE_DATA* data);

	inline int w() const							{ return _w; } 
	inline int h() const							{ return _h; }
	inline int nc() const							{ return _nc; }
	inline int snc() const							{ return _snc; }
	inline const unsigned char* getData() const		{ return _mdata.get(); }	//data can't be modified, only read
	inline const std::string& getName() const		{ return _fileName; }
	inline ID3D11ShaderResourceView* getSRV()		{ return _srv; }

	inline void freeMemory() { if (_mdata.get()) _mdata.reset(); }

	static std::vector<float> LoadAsFloatVec(const std::string& path);
	static void SaveAsPng(const char* targetFile, int w, int h, int comp, const void* data, int stride_in_bytes = 0u);

	//easier desc creation (hopefully... textures aren't quite so uniformly created in general)
	static inline D3D11_TEXTURE2D_DESC create2DTexDesc(
		UINT w,
		UINT h,
		DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE,
		UINT cpuAccessFlags = 0u,	//D3D11_CPU_ACCESS_WRITE (dynamic or staging), D3D11_CPU_ACCESS_READ	(staging)
		UINT miscFlags = 0u,
		UINT mipLevels = 1u,
		UINT arraySize = 1u,
		DXGI_SAMPLE_DESC sdcq = { 1, 0 })
	{
		return D3D11_TEXTURE2D_DESC{ w, h, mipLevels, arraySize, format, sdcq, usage, bindFlags, cpuAccessFlags, miscFlags };
	}


	static std::vector<unsigned char> loadToSysMem(ID3D11Device* device, ID3D11DeviceContext* dc, const Texture& tex)
	{
		// Create a staging texture to copy to, currently not the entire format is stored in texture so I only use number of channels
		ID3D11Texture2D* stagingId = nullptr;

		D3D11_TEXTURE2D_DESC texDesc{}; //Texture::create2DTexDesc(_w, _h, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_USAGE_STAGING, 0u, D3D11_CPU_ACCESS_READ, 0u);
		texDesc.Width = tex.w();
		texDesc.Height = tex.h();
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = N_TO_FORMAT_DX11[tex._nc - 1];
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_STAGING;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		texDesc.BindFlags = 0;
		texDesc.MiscFlags = 0;

		if (FAILED(device->CreateTexture2D(&texDesc, 0, &stagingId)))
		{
			OutputDebugStringA("Can't create off-screen texture. \n");
			exit(425);
		}

		// Copy data from the GPU texture to the staging texture
		dc->CopyResource(stagingId, tex._dxID);

		D3D11_MAPPED_SUBRESOURCE msr;
		dc->Map(stagingId, 0, D3D11_MAP_READ, 0, &msr);

		unsigned char* pDataPtr = static_cast<unsigned char*>(msr.pData);

		std::vector<unsigned char> result(pDataPtr, pDataPtr + msr.DepthPitch);
		
		dc->Unmap(stagingId, 0);

		stagingId->Release();

		return result;
	}


	// This is slow, intended for debugging
	static void saveToFile(ID3D11Device* device, ID3D11DeviceContext* dc, const Texture& tex, const char* filepath)
	{
		auto imageData = loadToSysMem(device, dc, tex);
		Texture::SaveAsPng(filepath, tex.w(), tex.h(), tex.nc(), imageData.data());
	}
};