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

	ID3D11Texture2D *_texPtr;
	ID3D11ShaderResourceView* _shResView;

public:

	Skybox() {}



	Skybox(ID3D11Device* device, std::string path, Model* model, Material* m, UINT resolution = 512u)
	{
		CubeMap::loadCubeMapFromFile(device, path, resolution, _texPtr, _shResView);
		_r = Renderable(model->_meshes[0]);
		_r.mat = m;
	}



	void renderSkybox(Camera& cam, Renderer& renderer)
	{
		D3D* d3d = renderer.d3d();
		ID3D11DeviceContext* context = renderer.context();

		d3d->setRSSolidNoCull();
		d3d->setDSSLessEquals();

		Math::SetTranslation(_r._transform, cam.GetPosition());
		
		//update and set cbuffers
		_r.updateBuffersAuto(context);
		_r.setBuffers(context);

		//set shaders and similar geebees
		context->VSSetShader(_r.mat->getVS()->_vsPtr, NULL, 0);
		context->PSSetShader(_r.mat->getPS()->_psPtr, NULL, 0);
		context->IASetInputLayout(_r.mat->getVS()->_layout);
		_r.mat->setSamplers(context);

		context->PSSetShaderResources(0, 1, &_shResView);

		context->IASetPrimitiveTopology(_r.mat->primitiveTopology);

		unsigned int stride = _r.mesh->_vertexBuffer._stride;
		unsigned int offset = _r.mesh->_vertexBuffer._offset;

		context->IASetVertexBuffers(0, 1, _r.mesh->_vertexBuffer.ptr(), &stride, &offset);
		context->IASetIndexBuffer(_r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(_r.mesh->_indexBuffer.getIdxCount(), 0, 0);

		ID3D11ShaderResourceView* unbinder[] = { nullptr };
		context->PSSetShaderResources(0, 1, unbinder);

		d3d->setDSSLess();
		d3d->setRSSolidCull();
	}
};