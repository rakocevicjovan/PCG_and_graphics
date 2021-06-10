#include "pch.h"
#include "ParticleSystem.h"
#include "Model.h"

class Phong;


ParticleSystem::ParticleSystem()
{
}


ParticleSystem::~ParticleSystem()
{
	for (auto* pb : _particles)
		delete pb;

	_particles.clear();
}



void ParticleSystem::init(Model* pModel, unsigned int particleCount, SMatrix transform)
{
	model = pModel;

	_numParticles = particleCount;

	pud = new ParticleUpdateData;

	for (int i = 0; i < _numParticles; ++i)
	{
		_particles.push_back(new ParticleBase());
	}
	
	for (auto& p : _particles) 
	{
		p->_ps = this;
	}

	_transform = transform;
}



void ParticleSystem::setShader(Phong* shader)
{
	_shader = shader;
}



void ParticleSystem::setUpdateFunction(std::function<void(ParticleUpdateData* pud)> particleUpdFunc)	//void(*funcPtr)(PUD *pud)
{
	updateStdFunc = particleUpdFunc;
}



void ParticleSystem::update(float deltaTime)
{
	pud->dTime = deltaTime;
	updateStdFunc(pud);
}



void ParticleSystem::draw(ID3D11DeviceContext* dc)
{
	//for (ParticleBase* p : _particles)
		//_model->Draw(dc, *_shader);
}