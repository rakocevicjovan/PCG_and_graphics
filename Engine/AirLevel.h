#pragma once
#include "Level.h"
#include "Dragon.h"


class AirLevel : public Level
{
public:

	PointLight pointLight;
	DirectionalLight dirLight;
	SMatrix lightView;
	Texture worley;

	Model skybox, barrens, headModel, segmentModel;
	CubeMapper skyboxCubeMapper;

	Dragon dragon;

	SVec3 windDir = SVec3(1, 0, 0);
	float windInt = 33.f;
	std::vector<InstanceData> instanceData;

	AirLevel(Systems& sys) : Level(sys) {};
	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void demolish() { this->~AirLevel(); };
};
