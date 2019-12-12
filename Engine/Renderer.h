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



//can be a bunch of packed bits but do I really need it to be? this class can produce an int64_t eventually
class RenderableQueueKey
{
public:
	unsigned char renderTarget;		//max 256 render targets...
	uint16_t shaderSetId;			//max 65536 shader combinations
	uint16_t textureId;				//max 65536 texture combinations
	uint16_t depth;					//see how to shring float into uint16_t easily...
	unsigned char vertexFormat;		//max 256 vertex formats

	int64_t create64bitKey()
	{
		int64_t result = 
			renderTarget << (63 - 8) |
			shaderSetId << (63 - 24) |
			textureId << (63 - 40) |
			depth << (63 - 56) |
			vertexFormat << (63 - 56);

		return result;
	}
};



class Renderer
{
private:

	bool UpdateRenderContext(float dTime);

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	ResourceManager* _resMan;
	D3D* _d3d;

	const size_t MAX_OPAQUES	  = 500;
	const size_t MAX_TRANSPARENTS = 100;
	//StackAllocator sAlloc;
	std::vector<Renderable> transparents;
	std::vector<Renderable> opaques;
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
	void render(const Renderable& renderable);
	void clearRenderQueue();

	void setCameraMatrix(const SMatrix& camMatrix);



	float _fieldOfView, _screenAspect, elapsed = 0.f;
};