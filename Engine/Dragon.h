#pragma once
#include "ParticleSystem.h"
#include "Systems.h"

class Dragon
{
public:

	Dragon() {}
	~Dragon() { delete dud; }

	void init(const Model& head, const Model& segment);
	void update(const RenderContext& rc, const SVec3& wind);
	void draw(const RenderContext& rc);
	DragonUpdateData* dud;

	Model _head, _segment;


	ParticleSystem pSys;
	std::function<void(ParticleUpdateData*)> particleUpdFunc;

};