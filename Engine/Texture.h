#pragma once

#include <string>
#include <d3d11.h>
#include <vector>
#include "assimp\scene.h"
#include "WICTextureLoader.h"

namespace Procedural
{
	class Perlin;
}



class Texture {

public:

	//width, height, channels and actual image data
	int w, h, n;
	unsigned char *data;

	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D* texId;
	
	D3D11_SUBRESOURCE_DATA texData;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ID3D11ShaderResourceView* srv;

	std::string typeName;
	std::string fileName;


	Texture(ID3D11Device*, const std::string& fileName);
	Texture::Texture();

	bool Load();
	bool LoadFromFile(std::string path);
	bool LoadFromMemory(const aiTexture *texture, ID3D11Device* device);
	bool LoadFromPerlin(ID3D11Device* device, Procedural::Perlin& perlin);
	bool Setup(ID3D11Device* device, bool grayscale = false);

	static std::vector<float> Texture::GetFloatsFromFile(const std::string& path);	//, std::vector<float>& target
	
	static void WriteToFile(const std::string& targetFile, int w, int h, int comp, void* data, int stride_in_bytes);

	static float Perlin3D(float x, float  y, float z, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static float Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static float Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static float Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	
	static std::vector<float> generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);
	static std::vector<float> generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves, UINT xw = 0, UINT yw = 0, UINT zw = 0);

	ID3D11ShaderResourceView* getTextureResourceView() { return srv; }

	void LoadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path);
};