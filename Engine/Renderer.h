#include "d3dclass.h"
#include "Camera.h"
#include "Model.h"
#include "ShaderManager.h"
#include "Rekt.h"
#include "OST.h"
#include "CubeMapper.h"
#include "GameClock.h"
#include "ParticleSystem.h"

//procedural
#include "Terrain.h"
#include "Perlin.h"
#include "LSystem.h"
#include "Voronoi.h"

#include "BitMapper.h"

//@TODO VERY IMPORTANT CLEAR THIS OUT OF HERE AND INTO THE SHADER MANAGER OR RESOURCE MANAGER!
#include "ShaderDataStructs.h"


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

	ShaderManager shMan;

	std::vector<Model*> _terrainModels;
	
	Texture NST, DST, white, perlinTex, worley;
	Camera _cam;
	Controller _controller;
	PointLight pointLight;
	DirectionalLight dirLight;

	OST offScreenTexture;
	Rekt* _rekt;
	Rekt::UINODE* screenRect;

	CubeMapper cubeMapper, shadowCubeMapper, skyboxCubeMapper;

	Model modTerrain, modTreehouse, modBall, modSkybox, modWaterQuad, modStrife, modDepths, modBallStand;

	Procedural::Terrain proceduralTerrain;
	Procedural::Perlin perlin;
	Procedural::LSystem linden;
	ParticleSystem pSys;

	bool drawUI;

	const unsigned int ostW = 1600, ostH = 900;

	float clearColour[4] = { 0.3f, 0.0f, 0.8f, 1.0f };

	float elapsed = 0.f;
	bool uwotm8 = false;
	

	ID3D11Buffer *_vertexBuffer, *_indexBuffer;

	std::function<void(PUD*)> lambda;
	std::function<void(PUD*)> lambda1;
	bool isTerGenerated = false;
};