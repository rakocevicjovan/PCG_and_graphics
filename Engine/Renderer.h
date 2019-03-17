#include "d3dclass.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "CollisionEngine.h"
#include "Rekt.h"
#include "CubeMapper.h"
#include "GameClock.h"
#include "ParticleSystem.h"


//procedural
#include "Terrain.h"
#include "Perlin.h"
#include "LSystem.h"
#include "Voronoi.h"
#include "Volumetric.h"
#include "BitMapper.h"
#include "Geometry.h"
#include "Maze.h"


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

	CubeMapper cubeMapper, shadowCubeMapper, skyboxCubeMapper;

	Procedural::Terrain proceduralTerrain;
	Procedural::Perlin perlin;
	Procedural::LSystem linden;
	Model treeModel;
	Procedural::Maze maze;

	ParticleSystem pSys;

	bool drawUI;

	float clearColour[4] = { 0.3f, 0.0f, 0.8f, 1.0f };

	float elapsed = 0.f;
	bool uwotm8 = false;

	std::function<void(PUD*)> lambda;
	std::function<void(PUD*)> lambda1;
	bool isTerGenerated = false;
};