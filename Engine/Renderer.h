#pragma once
#include "D3D.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "RenderStateManager.h"
#include "StackAllocator.h"
#include "ClusterManager.h"
#include "RenderStage.h"
#include "ReservedBuffers.h"
#include "Pausable.h"

// @TODO make data driven and add a menu to set them...
inline bool FULL_SCREEN = false;
inline bool VSYNC_ENABLED = true;
inline float FAR_PLANE = 1000.0f;
inline float NEAR_PLANE = 1.0f;

// This can be smarter, vary based on needs, but that's a story for another day
static std::array<UINT, 3> CLUSTER_GRID_DIMS{ 30, 17, 16 };

class Renderable;


// Delete, should exist in some sense but not like this.
struct RenderContext
{
	D3D* d3d{};
	float dTime{};
	float elapsed{};
	Camera* cam{};
};


class Renderer : public Pausable<Renderer>
{
private:

	float _windowWidth{ 0.f };
	float _windowHeight{ 0.f };
	float _fieldOfView{ 0.f };
	float _aspectRatio{ 0.f };
	float _elapsed{ 0.f };

	ID3D11Device* _device{};
	ID3D11DeviceContext* _deviceContext{};
	D3D* _d3d{};

	CBuffer _perCamBuffer;
	CBuffer _VSperFrameBuffer;
	CBuffer _PSperFrameBuffer;

	void updateRenderContext(float dTime);
	void createGlobalBuffers();

public:

	Camera _cam;
	RenderQueue _rQue;

	std::unique_ptr<ClusterManager> _clusterManager;

	RenderContext rc;

	Renderer();
	~Renderer();

	bool initialize(int wWidth, int wHeight, D3D& d3d);
	void resize(uint16_t width, uint16_t height);
	void frame(float dTime);
	void updatePerFrameBuffers(float dTime);
	void updatePerCamBuffer(float ww, float wh);
	
	void setDefaultRenderTarget();

	inline void addToRenderQueue(Renderable& renderable) { _rQue.insert(renderable); }
	inline void sortRenderQueue() { _rQue.sort(); }
	void flushRenderQueue();
	inline void clearRenderQueue() { _rQue.clear(); };

	void render(const Renderable& renderable) const;

	inline float getAspectRatio() { return _aspectRatio; }

	inline ID3D11Device* device () { return _device; }
	inline ID3D11DeviceContext* context () { return _deviceContext; }
	inline D3D* d3d () { return _d3d; }

	inline void doWork(float dTime)
	{
		frame(dTime);
	}
};