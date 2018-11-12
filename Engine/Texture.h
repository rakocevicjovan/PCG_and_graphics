#pragma once

#include <string>
#include <d3d11.h>
#include "assimp\scene.h"    

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

	//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc; could be useful

	//whether it's diffuse, specular etc. for convenience
	std::string typeName;

	//where we got the texture
	std::string fileName;


	Texture(ID3D11Device*, const std::string& fileName);
	Texture::Texture();

	bool Load();
	bool LoadFromFile(std::string path);
	bool LoadFromMemory(const aiTexture *texture, ID3D11Device* device);
	bool Setup(ID3D11Device* device);
	static void WriteToFile(const std::string& targetFile, int w, int h, int comp, void* data, int stride_in_bytes);
	
	ID3D11ShaderResourceView* getTextureResourceView() { return srv; }
};