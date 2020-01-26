#pragma once
#include <string>
#include <d3d11.h>
#include <vector>
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
	unsigned char *_data;

public:
	//needs to be retained for GPU use
	ID3D11Texture2D* texId;
	ID3D11ShaderResourceView* srv;

	std::string _fileName;	//helpful to debug loaders with but otherwise meh... 
	
	//static const size_t NUM_ROLES = 8u;
	TextureRole _role;
	std::string _typeName;	//type should be a part of Material definition when that's working (I think...)


	Texture(ID3D11Device* device, const std::string& fileName);
	Texture(const std::string& fileName);
	Texture();

	bool LoadFromStoredPath();
	bool LoadFromFile(std::string path);
	bool LoadFromMemory(const unsigned char* texture, size_t size);
	void LoadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path);
	bool LoadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin);

	bool Setup(ID3D11Device* device, DXGI_FORMAT f = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

	static void WriteToFile(const std::string& targetFile, int w, int h, int comp, void* data, int stride_in_bytes);
	
	ID3D11ShaderResourceView* getTextureResourceView() { return srv; }

	inline int getW() const { return w; } 
	inline int getH() const { return h; }
	inline int getN() const { return n; }
	inline const unsigned char* getData() const { return _data; }	//data can't be modified, only read


	//this really, really, does not belong here...
	static std::vector<float> Texture::LoadAsFloatVec(const std::string& path);

protected:

	//shouldn't be here but not sure if I can call stb functions outside of texture cpp... solved with friend class
	inline static float Perlin3D(float x, float  y, float z, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	inline static float Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	inline static float Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	inline static float Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);

	static std::vector<float> generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static std::vector<float> generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
};