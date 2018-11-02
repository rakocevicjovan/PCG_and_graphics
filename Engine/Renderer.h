#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include "d3dclass.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "ShaderHUD.h"
#include "ShaderDepth.h"
#include "ShaderWireframe.h"
#include "ShaderPT.h"
#include "ShaderShadow.h"
#include "Rekt.h"
#include "OST.h"



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

	Camera& addCamera(SMatrix& camTransform, SMatrix& lens);
	Shader& addShader();

private:
	bool RenderFrame(const std::vector<Model*>& m, const Camera& cam);

	D3DClass* _D3D;
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

	std::vector<Camera> _cameras;
	std::vector<Shader> _shaders;
	WireframeShader wfs;
	ShaderHUD shaderHUD;
	ShaderDepth shaderDepth;
	ShaderPT shaderPT;
	ShaderShadow shaderShadow;
	std::vector<Model*> _models;
	std::vector<Controller> _controllers;	//@TODO Reorganize this as well! Renderer should not hold controllers and models!
	std::vector<DirectionalLight> _lights;
	PointLight pLight;
	SMatrix _ostpm;
	OST offScreenTexture;
	Rekt* _rekt;
	Rekt::UINODE* screenRect;

	float clearColour[4] = { 0.3f, 0.0f, 0.8f, 1.0f };
	float ccb[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	ID3D11Buffer *_vertexBuffer, *_indexBuffer;
	Model mod, mod2;	//@TODO REMOVE WHEN YOU HAVE AN ACTUAL STORAGE
};
#endif