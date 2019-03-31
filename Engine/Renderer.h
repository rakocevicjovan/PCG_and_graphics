#include "d3dclass.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "CollisionEngine.h"
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

	bool Initialize(int, int, HWND, InputManager& inMan);
	bool Frame(float dTime);
	void Shutdown();
	void ProcessSpecialInput();

private:

	bool RenderFrame(float dTime);
	void OutputFPS(float dTime);

	D3DClass* _D3D;
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

	InputManager* _inMan;
	ResourceManager _resMan;
	ShaderManager shMan;
	CollisionEngine _colEngine;

	Camera _cam;
	Controller _controller;

	Rekt* _rekt;
	Rekt::UINODE* screenRect;

	Level* currentLevel;

	bool drawUI;

	float clearColour[4] = { 0.3f, 0.0f, 0.8f, 1.0f };

	float elapsed = 0.f;
};