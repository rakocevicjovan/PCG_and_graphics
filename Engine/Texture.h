#pragma once
#include <wrl/client.h>
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
	
protected:
	
	// width, height, max 65536 maximum each
	uint16_t _w{ 0u };
	uint16_t _h{ 0u };

	// num channels
	uint8_t _nc{ 0u };

	std::shared_ptr<unsigned char[]> _mdata;

	static int GetFormatFromFile(const char* filename);
	static int GetFormatFromMemory(const unsigned char* data, size_t size);

public:

	Microsoft::WRL::ComPtr<ID3D11Texture2D> _dxID{};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _arraySrv{};

	std::string _fileName;	// Helpful to debug, likely to be removed in release...

	Texture();
	Texture(ID3D11Device* device, const std::string& fileName);
	Texture(const std::string& fileName);

	bool loadFromFile(const char* filename);
	bool loadFromMemory(const unsigned char* data, size_t size);
	
	bool loadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path);
	bool loadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin);

	bool setUpAsResource(ID3D11Device* device, bool deleteData = true);
	bool createGPUResource(ID3D11Device* device, D3D11_TEXTURE2D_DESC* desc, D3D11_SUBRESOURCE_DATA* data);
	bool createSRV(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc);

	inline void setWidth(UINT width)  { _w = width; }
	inline void setHeight(UINT height) { _h = height; }

	inline int w() const							{ return _w; } 
	inline int h() const							{ return _h; }
	inline int nc() const							{ return _nc; }
	inline const unsigned char* getData() const		{ return _mdata.get(); }	//data can't be modified, only read
	inline const std::string& getName() const		{ return _fileName; }
	inline ID3D11ShaderResourceView* getSRV()		{ return _arraySrv.Get(); }

	inline void freeMemory() { if (_mdata.get()) _mdata.reset(); }

	static std::vector<float> LoadAsFloatVec(const std::string& path);
	static void SaveAsPng(const char* targetFile, int w, int h, int comp, const void* data, int stride_in_bytes = 0u);

	// Easier desc creation (hopefully... textures aren't quite so uniformly created in general)
	static inline D3D11_TEXTURE2D_DESC create2DTexDesc(
		UINT w,
		UINT h,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
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


	static std::vector<unsigned char> LoadToSysMem(ID3D11Device* device, ID3D11DeviceContext* dc, const Texture& tex)
	{
		// Create a staging texture to copy to, currently not the entire format is stored in texture so I only use number of channels
		Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture{};

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

		if (FAILED(device->CreateTexture2D(&texDesc, 0, stagingTexture.GetAddressOf())))
		{
			OutputDebugStringA("Can't create off-screen texture. \n");
			exit(425);
		}

		// Copy data from the GPU texture to the staging texture
		dc->CopyResource(stagingTexture.Get(), tex._dxID.Get());

		D3D11_MAPPED_SUBRESOURCE msr;
		dc->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &msr);

		unsigned char* pDataPtr = static_cast<unsigned char*>(msr.pData);

		std::vector<unsigned char> result(pDataPtr, pDataPtr + msr.DepthPitch);
		
		dc->Unmap(stagingTexture.Get(), 0);

		return result;
	}


	// This is slow, intended for debugging
	static void SaveToFile(ID3D11Device* device, ID3D11DeviceContext* dc, const Texture& tex, const char* filepath)
	{
		auto imageData = LoadToSysMem(device, dc, tex);
		Texture::SaveAsPng(filepath, tex.w(), tex.h(), tex.nc(), imageData.data());
	}
};