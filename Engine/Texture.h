#pragma once

#include <string>
#include <d3d11.h>
#include <vector>
#include "WICTextureLoader.h"
#include "Resource.h"
#include "StackAllocator.h"



namespace Procedural
{
	class Perlin;
}



class Texture : public Resource
{
private:
	//width, height, channels and actual image data
	//doesn't have to be retained after loading unless we need to operate on the texture on the CPU side
	int w, h, n;
	unsigned char *_data;

public:
	//needs to be retained for GPU use
	ID3D11Texture2D* texId;
	ID3D11ShaderResourceView* srv;

	std::string _fileName;	//helpful to debug loaders with but otherwise meh... 
	std::string typeName;	//type should be a part of Material definition when that's working, not here

	Texture(ID3D11Device*, const std::string& fileName);
	Texture(const std::string& fileName);
	Texture();

	bool LoadFromStoredPath();
	bool LoadFromFile(std::string path);
	bool LoadFromMemory(const unsigned char* texture, size_t size, ID3D11Device* device);
	bool LoadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin);
	void LoadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path);
	bool Setup(ID3D11Device* device, bool grayscale = false);

	static std::vector<float> Texture::GetFloatsFromFile(const std::string& path);	//, std::vector<float>& target
	
	static void WriteToFile(const std::string& targetFile, int w, int h, int comp, void* data, int stride_in_bytes);
	
	static std::vector<float> generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static std::vector<float> generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);

	ID3D11ShaderResourceView* getTextureResourceView() { return srv; }

	inline int getW() const { return w; } 
	inline int getH() const { return h; }
	inline int getN() const { return n; }
	inline const unsigned char* const getData() const { return _data; }	//ptr or data can't be modified, only read

	static float Perlin3D(float x, float  y, float z, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static float Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static float Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static float Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);

	/*void* operator new(size_t size, StackAllocator& stackAllocator)
	{
		assert(false);
		Texture* tex = ::new (stackAllocator.getHeadPtr()) Texture();
		return stackAllocator.alloc(sizeof(Texture));
	}*/

};