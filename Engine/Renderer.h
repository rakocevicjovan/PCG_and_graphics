#pragma once
#include "D3D.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "RenderStateManager.h"
#include "StackAllocator.h"
#include "ClusterManager.h"
#include "RenderStage.h"
#include "ReservedBuffers.h"
#include <memory>


// @TODO make data driven and add a menu to set them...
static bool FULL_SCREEN = false;
static bool VSYNC_ENABLED = true;
static float FAR_PLANE = 1000.0f;
static float NEAR_PLANE = 1.0f;

// This can be smarter, vary based on needs, but that's a story for another day
static std::array<UINT, 3> CLUSTER_GRID_DIMS = { 30, 17, 16 };

class Renderable;


// Delete, should exist in some sense but not like this.
struct RenderContext
{
	D3D* d3d;
	float dTime;
	float elapsed;
	Camera* cam;
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

	CBuffer _perCamBuffer;
	CBuffer _VSperFrameBuffer;
	CBuffer _PSperFrameBuffer;

	void updateRenderContext(float dTime);
	bool createGlobalBuffers();

public:
	Camera _cam;
	RenderQueue _rQue;

	std::unique_ptr<ClusterManager> _clusterManager;

	RenderContext rc;

	std::vector<RenderStage> _stages;

	Renderer();
	~Renderer();

	bool initialize(int wWidth, int wHeight, D3D& d3d);
	bool frame(float dTime);
	bool updatePerFrameBuffers(float dTime);
	
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