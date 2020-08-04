#pragma once
#include <string>
#include <d3d11.h>
#include <vector>
#include <memory>
#include "Resource.h"
#include "TextureRole.h"



namespace Procedural
{
	class Perlin;
	class TextureGen;
}


class Texture : public Resource
{
	friend class Procedural::TextureGen;
protected:
	//width, height, channels and actual image data
	//doesn't have to be retained after loading unless we need to operate on the texture on the CPU side
	int w, h, n;
	
	//unsigned char* _data;	with this a texture is 64 bytes so probably should use eventually...
	std::shared_ptr<unsigned char> _mdata;

public:
	//needs to be retained for GPU use
	ID3D11Texture2D* _dxID;
	ID3D11ShaderResourceView* _srv;

	std::string _fileName;	//helpful to debug loaders with but otherwise meh... 

	template <typename Archive> 
	void serialize(Archive& archive)
	{
		// Filename because I have an idea about it...
		archive(w, h, n, _mdata, filename);
	}

	Texture();
	Texture(ID3D11Device* device, const std::string& fileName);
	Texture(const std::string& fileName);
	Texture(const Texture& other);	// delete eventually?
	Texture(Texture&& other);
	Texture& Texture::operator=(const Texture& rhs);
	~Texture();

	bool LoadFromStoredPath();
	bool LoadFromFile(std::string path);
	bool LoadFromMemory(const unsigned char* texture, size_t size);
	void LoadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path);
	bool LoadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin);

	bool SetUpAsResource(ID3D11Device* device, DXGI_FORMAT f = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

	static void WriteToFile(const std::string& targetFile, int w, int h, int comp, void* data, int stride_in_bytes);
	
	ID3D11ShaderResourceView* getTextureResourceView() { return _srv; }

	inline int getW() const { return w; } 
	inline int getH() const { return h; }
	inline int getN() const { return n; }
	inline const unsigned char* getData() const { return _mdata.get(); }	//data can't be modified, only read
	inline std::string getName() const { return _fileName; }

	inline void freeMemory() { if (_mdata.get()) _mdata.reset(); }


	//weird...
	static std::vector<float> Texture::LoadAsFloatVec(const std::string& path);


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
	inline static float Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	inline static float Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	inline static float Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);

	static std::vector<float> generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static std::vector<float> generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
};