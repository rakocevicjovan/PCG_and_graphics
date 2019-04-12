#include "ParticleSystem.h"
#include "Model.h"



ParticleSystem::ParticleSystem()
{
}


ParticleSystem::~ParticleSystem()
{
	for (auto* pb : _particles)
		delete pb;

	_particles.clear();
}



void ParticleSystem::init(Model* pModel, unsigned int particleCount, SVec3 position)
{
	_model = pModel;

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



void ParticleSystem::setShader(ShaderBase* shader)
{
	_shader = shader;
}



void ParticleSystem::setUpdateFunction(std::function<void(ParticleUpdateData* pud)> particleUpdFunc)	//void(*funcPtr)(PUD *pud)
{
	updateStdFunc = particleUpdFunc;
}



void ParticleSystem::update(float deltaTime)
{
	for (ParticleBase* p : _particles)
		p->_ps->updateStdFunc(&pud);
}



void ParticleSystem::draw(ID3D11DeviceContext* dc)
{
	for (ParticleBase* p : _particles)
		_model->Draw(dc, *_shader);
}