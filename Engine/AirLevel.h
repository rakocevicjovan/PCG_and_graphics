#pragma once
#include "Level.h"
#include "Dragon.h"


class AirLevel : public Level
{
public:

	PointLight pointLight;
	DirectionalLight dirLight;
	SMatrix lightView;
	Texture worley, dragonTex;

	Player player;

	Procedural::Terrain barrensTerrain;
	Model skybox, barrens, glider, segmentModel, tornado, will;
	CubeMapper skyboxCubeMapper;

	OST windPipeTexture;
	ScreenspaceDrawer windPiper;
	ScreenspaceDrawer::UINODE* screenRectangleNode;

	Dragon dragon;
	DirectX::XMMATRIX gales;

	SVec3 windDir = SVec3(1, 0, 0);
	float windInt = 33.f;
	std::vector<InstanceData> instanceData;

	AirLevel(Systems& sys) : Level(sys), player(sys._controller) {};
	~AirLevel() {};
	void init(Systems& sys);
	void procGen() {};
	void update(const RenderContext& rc);
	void draw(const RenderContext& rc);
	void demolish() { this->~AirLevel(); };
};