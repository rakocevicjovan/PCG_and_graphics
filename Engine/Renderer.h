#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include "d3dclass.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "lightclass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class InputManager;

class Renderer {

public:
	Renderer();
	~Renderer();

	bool Initialize(int, int, HWND, InputManager& inMan);
	bool Frame();
	void Shutdown();

	Camera& addCamera(SMatrix& camTransform);
	Shader& addShader();

private:
	bool RenderFrame(const std::vector<Model*>& m, const Camera& cam, Shader& shader);


private:
	D3DClass* _D3D;
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

	std::vector<Camera> _cameras;
	std::vector<Shader> _shaders;
	std::vector<Model*> _models;
	std::vector<Controller> _controllers;	//@TODO Reorganize this as well! Renderer should not hold controllers and models!
	std::vector<DirectionalLight> _lights;
	SMatrix _projectionMatrix;
	
	ID3D11Buffer *_vertexBuffer, *_indexBuffer;
	Model mod, mod2;	//@TODO REMOVE WHEN YOU HAVE AN ACTUAL STORAGE

};
#endif