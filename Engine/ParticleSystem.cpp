#include "ParticleSystem.h"



ParticleSystem::ParticleSystem()
{
}


ParticleSystem::~ParticleSystem()
{
	for (auto* p : _particles)
		delete p;

	_particles.clear();
}



void ParticleSystem::init(ID3D11Device* device, unsigned int particleCount, SVec3 position, std::string& pathToModel)
{
	Model m;
	m.LoadModel(device, pathToModel);
	_models.push_back(m);

	_numParticles = particleCount;
	for (int i = 0; i < _numParticles; ++i)
	{
		_particles.push_back(new ParticleBase());
	}
	
	for (auto& p : _particles) 
	{
		p->_ps = this;
	}

	_position = position;
}



void ParticleSystem::setUpdateFunction(void(*funcPtr)(PUD *pud))
{
	updateStdFunc = funcPtr;
;}



void ParticleSystem::update(float deltaTime)
{
	for (ParticleBase* p : _particles)
		p->_ps->updateStdFunc(&pud);
}



void ParticleSystem::draw(ID3D11DeviceContext* dc)
{
	for (ParticleBase* p : _particles)
		p->draw(dc);
}