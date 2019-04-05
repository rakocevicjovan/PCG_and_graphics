#include "D3D.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "GameClock.h"
#include "Rekt.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class ResourceManager;
class InputManager;

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
public:
	Renderer();
	~Renderer();

	bool Initialize(int, int, HWND, ResourceManager& resMan, D3D& d3d, Controller& ctrl);
	bool Frame(float dTime, InputManager* inMan);

	float _fieldOfView, _screenAspect, elapsed = 0.f, sinceInput = 1.f;
	bool drawUI = false;
	Camera _cam;

	Rekt* _rekt;
	Rekt::UINODE* screenRect;

	RenderContext rc;

private:
	
	bool UpdateRenderContext(float dTime);

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

	ResourceManager* _resMan;
	ShaderManager _shMan;
	D3D* _d3d;
};