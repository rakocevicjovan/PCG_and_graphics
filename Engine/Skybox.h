#pragma once
#include "Renderer.h"

class Skybox
{
private:
	std::shared_ptr<Material> _mat;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> _texPtr{};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _shResView{};

public:

	Skybox() = default;

	Skybox(ID3D11Device* device, std::string_view path, std::shared_ptr<Material> material, UINT resolution = 512u);

	void renderSkybox(Camera& cam, Renderer& renderer);
};