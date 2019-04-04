#include "D3D.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "Rekt.h"
#include "GameClock.h"


const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class InputManager;

class Renderer {

public:
	Renderer();
	~Renderer();

	bool Initialize(int, int, HWND, InputManager& inMan, ResourceManager& resMan, D3D& d3d);
	bool Frame(float dTime);
	void ProcessSpecialInput(float dTime);

	float _fieldOfView, _screenAspect;

	Camera _cam;

private:

	bool RenderFrame(float dTime);
	void OutputFPS(float dTime);

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

	InputManager* _inMan;
	ResourceManager* _resMan;
	ShaderManager shMan;
	D3D* _d3d;

	Rekt* _rekt;
	Rekt::UINODE* screenRect;
	
	Level* _currentLevel;
	RenderContext rc;
	bool drawUI;
	float elapsed = 0.f;
	float sinceLastInput = 0.f;
};