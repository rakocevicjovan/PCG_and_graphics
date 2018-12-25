#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include "d3dclass.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "ShaderDepth.h"
#include "ShaderShadow.h"
#include "ShaderHUD.h"
#include "ShaderWireframe.h"
#include "ShaderPT.h"
#include "ShaderCM.h"
#include "ShaderSkybox.h"
#include "ShaderStrife.h"
#include "ShaderWater.h"
#include "Rekt.h"
#include "OST.h"
#include "CubeMapper.h"
#include "GameClock.h"

#include "BitMapper.h"


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
	bool Frame(float dTime);
	void Shutdown();

	Camera& addCamera(SMatrix& camTransform, SMatrix& lens);
	Shader& addShader();
	

private:

	bool RenderFrame(float dTime);
	void OutputFPS(float dTime);

	D3DClass* _D3D;
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

	std::vector<Camera> _cameras;
	std::vector<Shader> _shaders;
	std::vector<Model*> _terrainModels;

	Shader shaderLight;
	WireframeShader shaderWireframe;
	ShaderHUD shaderHUD;
	ShaderDepth shaderDepth;
	ShaderPT shaderPT;
	ShaderShadow shaderShadow;
	ShaderCM shaderCM;
	ShaderSkybox shaderSkybox;
	ShaderStrife shaderStrife;
	ShaderWater shaderWater;

	Texture NST, DST, white, perlin, worley;
	std::vector<Controller> _controllers;	//@TODO Reorganize this as well! Renderer should not hold controllers and models!
	PointLight pointLight;
	DirectionalLight dirLight;

	OST offScreenTexture;
	Rekt* _rekt;
	Rekt::UINODE* screenRect;
	D3D11_VIEWPORT altViewport;

	CubeMapper cubeMapper, shadowCubeMapper, skyboxCubeMapper;

	Model modTerrain, modTreehouse, modBall, modSkybox, modWaterQuad, modStrife, modDepths, modBallStand;

	bool drawUI;

	const unsigned int ostW = 1600, ostH = 900;

	float clearColour[4] = { 0.3f, 0.0f, 0.8f, 1.0f };
	float ccb[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	ID3D11Buffer *_vertexBuffer, *_indexBuffer;
};
#endif