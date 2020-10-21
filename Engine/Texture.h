#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Resource.h"
#include "TextureRole.h"

#include <d3d11_4.h>
#include <dxgiformat.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
//#include <cereal/types/memory.hpp>
//#include <cereal/access.hpp>


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

	//needs to be retained for GPU use
	ID3D11Texture2D* _dxID;
	ID3D11ShaderResourceView* _srv;

	std::string _fileName;	//helpful to debug loaders with but otherwise meh... 

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
	void loadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path);
	bool loadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin);

	bool setUpAsResource(ID3D11Device* device, bool deleteData = true);
	bool create(ID3D11Device* device, D3D11_TEXTURE2D_DESC* desc, D3D11_SUBRESOURCE_DATA* data);

	static void SaveAsPng(const std::string& targetFile, int w, int h, int comp, const void* data, int stride_in_bytes = 0u);
	
	std::pair<std::unique_ptr<unsigned char[]>, UINT> writeToMem();

	ID3D11ShaderResourceView* getTextureResourceView() { return _srv; }

	inline int w() const { return _w; } 
	inline int h() const { return _h; }
	inline int nc() const { return _nc; }
	inline int snc() const { return _snc; }
	inline const unsigned char* getData() const { return _mdata.get(); }	//data can't be modified, only read
	inline std::string getName() const { return _fileName; }

	inline void freeMemory() { if (_mdata.get()) _mdata.reset(); }


	//weird...
	static std::vector<float> LoadAsFloatVec(const std::string& path);


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
		//might want to ZeroMemory(&texDesc, sizeof(texDesc)), D3D11_RESOURCE_MISC_FLAG enum for reference
		return D3D11_TEXTURE2D_DESC{ w, h, mipLevels, arraySize, format, sdcq, usage, bindFlags, cpuAccessFlags, miscFlags };
	}



protected:	//delegated procedural generation interface to friend class
	
	inline static float Perlin3D(float x, float  y, float z, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	inline static float Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves);
	inline static float Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves);
	inline static float Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves);

	static std::vector<float> generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves);
	static std::vector<float> generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves);
};

/* 
// Unnecessary, png is a great format in it's own right 
// and anything I might need is possible to infer from it from now,
// while leaving images accessible without a custom editor
template <typename Archive>
void save(Archive& ar)
{
	// Tried this way, but raw textures are WAY bigger, png is really doing work
	//ar(_w, _h, _nc);
	//ar(cereal::binary_data(_mdata.get(), _w * _h * _nc));

	// Filename could be serialized for hot reload.

	auto compressedData = writeToMem();
	SaveAsPng()
		ar(cereal::binary_data(compressedData.first.get(), compressedData.second));
}

template <typename Archive>
void load(Archive& ar)
{
	// Filename could be serialized for hot reload.
	ar(_w, _h, _nc);
	std::unique_ptr<unsigned char[]> compressedData;
	ar(cereal::binary_data(compressedData.get(), _w * _h * _nc));
	_mdata = std::shared_ptr<unsigned char[]>(loadFromMemory(ar))
}
*/