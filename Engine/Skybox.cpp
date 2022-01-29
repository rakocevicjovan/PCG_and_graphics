#include "pch.h"

#include "Skybox.h"
#include "Camera.h"
#include "Model.h"
#include "CubeMap.h"
#include "Renderer.h"


Skybox::Skybox(ID3D11Device* device, std::string_view path, std::shared_ptr<Material> material, UINT resolution)
{
	CubeMap::LoadCubeMapFromFile(device, path, resolution, _texPtr.GetAddressOf(), _shResView.GetAddressOf());
	_mat = std::move(material);
}


void Skybox::renderSkybox(Camera& cam, Renderer& renderer)
{
	D3D* d3d = renderer.d3d();
	ID3D11DeviceContext* context = renderer.context();

	_mat->getVS()->bind(context);
	_mat->getPS()->bind(context);

	context->PSSetShaderResources(0, 1, _shResView.GetAddressOf());

	context->Draw(3, 0);

	thread_local ID3D11ShaderResourceView* unbinder{ nullptr };
	context->PSSetShaderResources(0, 1, &unbinder);
}