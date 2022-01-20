#pragma once
#include "Camera.h"
#include "Model.h"
#include "CubeMap.h"
#include "Renderer.h"

class Skybox
{
private:
	Material* _skyboxMaterial;
	Renderable _r;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> _texPtr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _shResView;

public:

	Skybox() {}



	Skybox(ID3D11Device* device, std::string_view path, Model* model, Material* m, UINT resolution = 512u)
	{
		CubeMap::LoadCubeMapFromFile(device, path, resolution, _texPtr.GetAddressOf(), _shResView.GetAddressOf());
		_r = Renderable(model->_meshes[0], SMatrix{});
		_r.mat = m;
	}



	void renderSkybox(Camera& cam, Renderer& renderer)
	{
		D3D* d3d = renderer.d3d();
		ID3D11DeviceContext* context = renderer.context();

		d3d->setRSSolidNoCull();
		d3d->setDSSLessEquals();

		Math::SetTranslation(_r._transform, cam.getPosition());
		
		//update and set cbuffers
		_r.mat->getVS()->_cbuffers[0].updateWithStruct(renderer.context(), _r._transform.Transpose());
		_r.setBuffers(context);

		//set shaders and similar geebees
		_r.mat->getVS()->bind(context);
		_r.mat->getPS()->bind(context);

		context->PSSetShaderResources(0, 1, _shResView.GetAddressOf());

		_r.mesh->_vertexBuffer.bind(context);
		_r.mesh->_indexBuffer.bind(context);

		context->DrawIndexed(_r.mesh->_indexBuffer.getIdxCount(), 0, 0);

		thread_local ID3D11ShaderResourceView* unbinder{ nullptr };
		context->PSSetShaderResources(0, 1, &unbinder);

		d3d->setDSSLess();
		d3d->setRSSolidCull();
	}
};