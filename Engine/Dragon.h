#pragma once
#include "ParticleSystem.h"
#include "Systems.h"

class Dragon
{
public:

	Dragon() {}
	~Dragon() {}



	void init(const Model& head, const Model& segment);

	Model _head, _segment;


	ParticleSystem pSys;
	std::function<void(ParticleUpdateData*)> particleUpdFunc;

};