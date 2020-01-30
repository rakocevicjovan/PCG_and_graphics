#pragma once
#include "Camera.h"
#include "Model.h"
#include "CubeMapper.h"
#include "Renderer.h"

class Skybox
{
private:
	CubeMapper _cubeMapper;
	Material* _skyboxMaterial;
	Renderable _r;

public:

	Skybox() {}

	Skybox(ID3D11Device* device, std::string path, Model* model, Material* m)
	{
		_cubeMapper.LoadFromFiles(device, path);
		_r = Renderable(model->meshes[0]);
		_r.mat->setVS(m->getVS());
		_r.mat->setPS(m->getPS());
	}

	void renderSkybox(Camera& cam, Renderer& renderer)
	{
		D3D* d3d = renderer.d3d();
		auto* context = renderer.context();

		d3d->setRSSolidNoCull();
		d3d->SwitchDepthToLessEquals();

		Math::SetTranslation(_r._transform, cam.GetPosition());
		
		//update and set cbuffers
		_r.updateBuffersAuto(context);
		_r.setBuffers(context);

		//set shaders and similar geebees
		context->VSSetShader(_r.mat->getVS()->_vsPtr, NULL, 0);
		context->PSSetShader(_r.mat->getPS()->_psPtr, NULL, 0);
		context->IASetInputLayout(_r.mat->getVS()->_layout);
		context->PSSetSamplers(0, 1, &_r.mat->getPS()->_sState);

		context->PSSetShaderResources(0, 1, &(_cubeMapper.cm_srv));

		context->IASetPrimitiveTopology(_r.mat->primitiveTopology);

		unsigned int stride = _r.mesh->_vertexBuffer._stride;
		unsigned int offset = _r.mesh->_vertexBuffer._offset;

		context->IASetVertexBuffers(0, 1, _r.mesh->_vertexBuffer.ptr(), &stride, &offset);
		context->IASetIndexBuffer(_r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(_r.mesh->indexCount, 0, 0);

		ID3D11ShaderResourceView* unbinder[] = { nullptr };
		context->PSSetShaderResources(0, 1, unbinder);

		d3d->SwitchDepthToDefault();
		d3d->setRSSolidCull();
	}
};