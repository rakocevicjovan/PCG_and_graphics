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

	Player gliderPlayer;

	Model skybox, barrens, glider, segmentModel;
	CubeMapper skyboxCubeMapper;

	OST windPipeTexture;
	ScreenspaceDrawer windPiper;
	ScreenspaceDrawer::UINODE* screenRectangleNode;

	Dragon dragon;
	DirectX::XMMATRIX gales;

	SVec3 windDir = SVec3(1, 0, 0);
	float windInt = 33.f;
	std::vector<InstanceData> instanceData;

	AirLevel(Systems& sys) : Level(sys), gliderPlayer(sys._controller) {};
	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void demolish() { this->~AirLevel(); };
};

/*
	gales.r[0] = SVec4(0, 100, 100, 13);
	gales.r[1] = SVec4(100, 100, 33, 15);
	gales.r[2] = SVec4(0, 100, 120, 12);
	gales.r[3] = SVec4(-170, 100, 15, 17);
*/