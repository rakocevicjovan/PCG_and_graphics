#pragma once
#include "D3D.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "RenderStateManager.h"

#include "OST.h"
#include "CubeMapper.h"
#include "ShaderManager.h"
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


struct PerCameraBuffer
{
	SMatrix proj;
};


struct PerFrameBuffer
{
	SMatrix mat;
	float delta;
	float elapsed;
	SVec2 padding;
};



class Renderer
{
private:

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	ResourceManager* _resMan;
	D3D* _d3d;
	//StackAllocator sAlloc;

	float _fieldOfView, _aspectRatio, elapsed = 0.f;
	ID3D11Buffer* _perCamBuffer;
	ID3D11Buffer* _perFrameBuffer;

	bool updateRenderContext(float dTime);
	bool createGlobalBuffers();

public:
	ShaderManager _shMan;
	RenderContext rc;
	Camera _cam;
	RenderQueue _rQue;

	Renderer();
	~Renderer();

	bool initialize(int wWidth, int wHeight, HWND wHandle, ResourceManager& resMan, D3D& d3d, Controller& ctrl);
	bool frame(float dTime, InputManager* inMan);
	bool updatePerFrameBuffer(float dTime);
	
	void setOSTRenderTarget(OST& ost);
	void setDefaultRenderTarget();

	void renderSkybox(const Camera& cam, Model& skybox, const CubeMapper& skyboxCubeMapper);

	inline void addToRenderQueue(Renderable& renderable) { _rQue.add(renderable); }
	inline void sortRenderQueue() { _rQue.sort(); }
	void flushRenderQueue();
	inline void clearRenderQueue() { _rQue.clear(); };

	void render(const Renderable& renderable);
	
	void setCameraMatrix(const SMatrix& camMatrix);

	inline float getAR() { return _aspectRatio; }
};