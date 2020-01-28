#pragma once
#include "Camera.h"
#include "Model.h"
#include "CubeMapper.h"
#include "Renderer.h"

class Skybox
{
private:
	Model* _skyboxModel;
	CubeMapper _cubeMapper;
	Material* _skyboxMaterial;
	Renderable _r;

public:

	Skybox() {}

	Skybox(ID3D11Device* device, std::string path, Model* model, Material* m) : _skyboxModel(model)
	{
		_cubeMapper.LoadFromFiles(device, path);
		_r = Renderable(model->meshes[0]);
		_r.mat = m;
	}

	void renderSkybox(Camera& cam, Renderer& renderer)
	{
		D3D* d3d = renderer.d3d();

		d3d->setRSSolidNoCull();
		d3d->SwitchDepthToLessEquals();

		_r.worldTransform = cam.GetCameraMatrix();
		renderer.render(_r);

		//skyboxShader.SetShaderParameters(renderer.context(), cam, rc.dTime, skyboxCubeMapper.cm_srv);

		d3d->SwitchDepthToDefault();
		d3d->setRSSolidCull();
	}
};