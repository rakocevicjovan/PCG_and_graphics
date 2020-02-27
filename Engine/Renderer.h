#pragma once
#include "D3D.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "RenderStateManager.h"
#include "OST.h"
#include "ScreenSpaceDrawer.h"
#include "StackAllocator.h"


// @TODO make data driven and add a menu to set them...
static bool FULL_SCREEN = false;
static bool VSYNC_ENABLED = true;
static float FAR_PLANE = 1000.0f;
static float NEAR_PLANE = 0.1f;


// System-reserved registers.
#define VS_PER_CAMERA_CBUFFER_REGISTER 10u
#define VS_PER_FRAME_CBUFFER_REGISTER 11u

#define PS_PER_FRAME_CBUFFER_REGISTER 10u


class Renderable;


struct RenderContext
{
	D3D* d3d;
	float dTime;
	float elapsed;
	Camera* cam;
};


struct VSPerCameraBuffer
{
	SMatrix proj;
};


struct VSPerFrameBuffer
{
	SMatrix viewMat;
	float delta;
	float elapsed;
	SVec2 padding;
};


struct PSPerFrameBuffer
{
	SVec4 eyePos;
	float elapsed;
	float delta;
	SVec2 padding;
};



class Renderer
{
private:

	float _fieldOfView;
	float _aspectRatio;
	float _elapsed = 0.f;

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	D3D* _d3d;

	ID3D11Buffer* VS_perCamBuffer;
	ID3D11Buffer* VS_perFrameBuffer;
	ID3D11Buffer* PS_perFrameBuffer;

	void updateRenderContext(float dTime);
	bool createGlobalBuffers();

public:
	RenderContext rc;
	Camera _cam;
	RenderQueue _rQue;

	Renderer();
	~Renderer();

	bool initialize(int wWidth, int wHeight, HWND wHandle, D3D& d3d);
	bool frame(float dTime);
	bool updatePerFrameBuffers(float dTime);
	
	void setOSTRenderTarget(OST& ost);
	void setDefaultRenderTarget();

	inline void addToRenderQueue(Renderable& renderable) { _rQue.add(renderable); }
	inline void sortRenderQueue() { _rQue.sort(); }
	void flushRenderQueue();
	inline void clearRenderQueue() { _rQue.clear(); };

	void render(const Renderable& renderable) const;
	
	void setCameraMatrix(const SMatrix& camMatrix);

	inline float getAspectRatio() { return _aspectRatio; }

	inline ID3D11Device* device () { return _device; }
	inline ID3D11DeviceContext* context () { return _deviceContext; }
	inline D3D* d3d () { return _d3d; }
};