#pragma once
#include "D3D.h"
#include "Camera.h"
#include "OST.h"
#include "CubeMapper.h"
#include "ShaderManager.h"
#include "GameClock.h"
#include "ScreenSpaceDrawer.h"
#include "StackAllocator.h"

const bool FULL_SCREEN = true;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class ResourceManager;
class InputManager;
class Renderable;

struct RenderContext
{
	D3D* d3d;
	float dTime;
	float elapsed;
	ShaderManager* shMan;
	Camera* cam;
};



class Renderer
{
private:

	bool UpdateRenderContext(float dTime);

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	ResourceManager* _resMan;
	D3D* _d3d;

	const size_t MAX_RENDERABLES = 500;
	//StackAllocator sAlloc;
	std::vector<Renderable> renderables;
	//std::vector<int64_t> qKeys;

public:
	ShaderManager _shMan;
	RenderContext rc;
	Camera _cam;


	Renderer();
	~Renderer();

	bool Initialize(int, int, HWND, ResourceManager& resMan, D3D& d3d, Controller& ctrl);
	bool Frame(float dTime, InputManager* inMan);
	
	void SetOSTRenderTarget(OST& ost);
	void RevertRenderTarget();

	void RenderSkybox(const Camera& cam, Model& skybox, const CubeMapper& skyboxCubeMapper);

	void addToRenderQueue(const Renderable& renderable);
	void sortRenderQueue();
	void flushRenderQueue();
	void render(Renderable& renderable);
	void clearRenderQueue();

	void setCameraMatrix(const SMatrix& camMatrix);



	float _fieldOfView, _screenAspect, elapsed = 0.f;
};