#pragma once
#include <vector>
#include "ParticleBase.h"


class ParticleSystem
{
protected:

	unsigned int _numParticles;
	std::vector<ParticleBase*> _particles;
	std::vector<Model> _models;
	PUD pud;
	SVec3 _position;

	void (ParticleBase::*Updaterino)(PUD* pud);
	std::function<void(PUD*)> updateStdFunc;

public:
	ParticleSystem();
	~ParticleSystem();

	void init(ID3D11Device* device, unsigned int particleCount, SVec3 position, std::string& meshPath);
	void setUpdateFunction(void(*funcPtr) (PUD* pud));
	void update(float deltaTime);
	void draw(ID3D11DeviceContext* dc);
};

