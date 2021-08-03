#pragma once
#include <wrl/client.h>
#include "TextureRole.h"

// This will be eventually used and expanded
enum class TexFormat : uint8_t
{
	R8,
	RG8,
	RGBA8,
	R32,
	RG32,
	RGBA32
};

class Image;


class Texture
{
protected:

	uint16_t _arrSize{ 1u };
	uint16_t _numMips{ 1u };

	// width, height, max 65536 maximum each
	uint16_t _w{ 0u };
	uint16_t _h{ 0u };
	
	uint8_t _nc{ 0u };
	DXGI_FORMAT _format{ DXGI_FORMAT_R8G8B8A8_UINT };

	// This should probably be removed and if a cpu copy is needed it should be kept in an Image instance instead...?
	std::shared_ptr<unsigned char[]> _mdata;

public:

	Microsoft::WRL::ComPtr<ID3D11Texture2D> _dxID{};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv{};

	Texture() = default;
	Texture(ID3D11Device* device, uint32_t w, uint32_t h, DXGI_FORMAT format, void* data, uint32_t flags = 0u, uint32_t mips = 1, uint32_t arrCount = 1);
	Texture(ID3D11Device* device, const std::string& fileName);

	bool loadFromImage(ID3D11Device* device, const Image& image, bool asSRV = true);
	bool loadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path);

	/// Quick ways to set up a texture from a file or memory by calling Image internally, image is discarded

	bool loadFromFile(ID3D11Device* device, const char* filename);
	bool loadFromMemory(ID3D11Device* device, const unsigned char* data, size_t size);
	
	void create(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* data, bool asSRV);
	void createSRV(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc);

	static std::vector<uint8_t> LoadToSysMem(ID3D11Device* device, ID3D11DeviceContext* dc, const Texture& tex);

	inline void setWidth(UINT width)  { _w = width; }
	inline void setHeight(UINT height) { _h = height; }

	inline int w() const							{ return _w; } 
	inline int h() const							{ return _h; }
	inline int nc() const							{ return _nc; }
	inline const unsigned char* getData() const		{ return _mdata.get(); }	// data can't be modified, only read
	inline ID3D11ShaderResourceView* getSRV()		{ return _srv.Get(); }

	inline void freeMemory() { if (_mdata.get()) _mdata.reset(); }

	// Functions below mostly abstract the graphics API. Probably shouldn't be directly in the class
	static Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture2D(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* data);
	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateSRV(ID3D11Device* device, ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc);
	static D3D11_SUBRESOURCE_DATA CreateSubresourceData(void* data, uint32_t textureWidth, uint32_t texelWidth, uint16_t mips = 1, uint16_t arrSize = 1);

	// Easier desc creation (hopefully... textures aren't quite so uniformly created in general)
	static inline D3D11_TEXTURE2D_DESC Create2DTexDesc(
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


	static constexpr inline DXGI_FORMAT TO_API_FORMAT(uint32_t format)
	{
		switch (format)
		{
		case 0: return DXGI_FORMAT_R8_UNORM;
		case 1: return DXGI_FORMAT_R8G8_UNORM;
		case 2: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case 3: return DXGI_FORMAT_R8G8B8A8_UNORM;
		default: assert(false);
		}
	}


	// Move this and similar items to api wrapper file(s)
	static constexpr inline DXGI_FORMAT TO_API_FORMAT(TexFormat format)
	{
		switch (format)
		{
		case TexFormat::R8: return DXGI_FORMAT_R8_UNORM;
		case TexFormat::RG8: return DXGI_FORMAT_R8G8_UNORM;
		case TexFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TexFormat::R32: return DXGI_FORMAT_R32_FLOAT;
		case TexFormat::RG32: return DXGI_FORMAT_R32G32_FLOAT;
		case TexFormat::RGBA32: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default: assert(false);
		}
	}

	static constexpr inline uint32_t FORMAT_TO_SIZE(DXGI_FORMAT format)
	{
		switch (format)
		{
		case TO_API_FORMAT(TexFormat::R8): return 1;
		case TO_API_FORMAT(TexFormat::RG8): return 2;
		case TO_API_FORMAT(TexFormat::RGBA8): return 4;
		case TO_API_FORMAT(TexFormat::R32): return 4;
		case TO_API_FORMAT(TexFormat::RG32): return 8;
		case TO_API_FORMAT(TexFormat::RGBA32): return 16;
		default: assert(false);
		}
	}
};