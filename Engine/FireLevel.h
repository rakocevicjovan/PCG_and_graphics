#pragma once
#include "Level.h"
#include "OST.h"
#include "Hexer.h"

class FireLevel : public Level
{
	Hexer hexer;
	float hexRadius = 30.f;

	OST sceneTex, brightnessMask, blurredTex1, blurredTex2;
	ScreenspaceDrawer postProcessor;
	ScreenspaceDrawer::UINODE* screenRectangleNode;

	Procedural::Terrain terrain, lavaSheet;
	Model skybox, lavaSheetModel, tree, islandModel;
	PointLight pointLight;
	CubeMapper skyboxCubeMapper;
	Model will, hexCluster, hexModel;
	Texture hexDiffuseMap, hexNormalMap;
	bool isTerGenerated = false;
	bool isFirst = true;

public:
	FireLevel(Systems& sys) : Level(sys), hexer(sys) {};
	~FireLevel() {};

	void init(Systems& sys);
	void procGen();
	void draw(const RenderContext& rc);
	void update(const RenderContext& rc);
	void demolish() { this->~FireLevel(); };
	bool processInput(float dTime);
	void setUpCollision();
	void resetCollision();
};