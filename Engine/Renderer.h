#pragma once
#include "D3D.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "RenderStateManager.h"
#include "OST.h"
#include "ScreenSpaceDrawer.h"
#include "StackAllocator.h"
#include "ClusterManager.h"
#include <memory>


// @TODO make data driven and add a menu to set them...
static bool FULL_SCREEN = false;
static bool VSYNC_ENABLED = true;
static float FAR_PLANE = 1000.0f;
static float NEAR_PLANE = 1.0f;

// This can be smarter, vary based on needs, but that's a story for another day
static std::array<UINT, 3> CLUSTER_GRID_DIMS = { 30, 17, 16 };


// System-reserved registers.

// For vertex shaders
#define VS_PER_CAMERA_CBUFFER_REGISTER 10u
#define VS_PER_FRAME_CBUFFER_REGISTER 11u

// For pixel shaders
#define PS_PER_CAMERA_CBUFFER_REGISTER 9u
#define PS_PER_FRAME_CBUFFER_REGISTER 10u

#define PS_CSM_CBUFFER_REGISTER 11u
#define PS_CSM_TEXTURE_REGISTER 11u

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


struct PSPerCameraBuffer
{
	float w;
	float h;
	float n;
	float f;
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

	float _windowWidth;
	float _windowHeight;
	float _fieldOfView;
	float _aspectRatio;
	float _elapsed = 0.f;

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	D3D* _d3d;

	ID3D11Buffer* _VSperCamBuffer;
	ID3D11Buffer* _VSperFrameBuffer;
	ID3D11Buffer* _PSperCamBuffer;
	ID3D11Buffer* _PSperFrameBuffer;

	void updateRenderContext(float dTime);
	bool createGlobalBuffers();

public:
	RenderContext rc;
	Camera _cam;
	RenderQueue _rQue;

	std::unique_ptr<ClusterManager> _clusterManager;

	Renderer();
	~Renderer();

	bool initialize(int wWidth, int wHeight, D3D& d3d);
	bool frame(float dTime);
	bool updatePerFrameBuffers(float dTime);
	
	void setOSTRenderTarget(OST& ost);
	void setDefaultRenderTarget();

	inline void addToRenderQueue(Renderable& renderable) { _rQue.insert(renderable); }
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