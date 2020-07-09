#pragma once
#include "Level.h"
#include "Math.h"
#include "Model.h"
#include "SkeletalModel.h"
#include "Light.h"

namespace SkelAnim
{

class SkelAnimTestLevel : public Level
{
public:
	SkelAnimTestLevel(Engine& sys) : Level(sys) {};

	void init(Engine& sys) override;
	void update(const RenderContext& rc) override;
	void draw(const RenderContext& rc) override;

private:

	Model floorModel;
	SkeletalModel sm;
	PointLight pLight;

	std::vector<SMatrix> bts;
};

}