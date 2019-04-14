#pragma once
#include <vector>
#include "ParticleBase.h"
#include "ShaderBase.h"

class Model;

class ParticleSystem
{
public:

	unsigned int _numParticles;
	Model* _model;
	ShaderBase* _shader;
	ParticleUpdateData* pud;
	SMatrix _transform;
	std::vector<ParticleBase*> _particles;

	ParticleSystem();
	~ParticleSystem();

	void init(Model* pModel, unsigned int particleCount, SMatrix transform);
	void setShader(ShaderBase* shader);
	void setUpdateFunction(std::function<void(ParticleUpdateData* pud)> particleUpdFunc);
	void update(float deltaTime);
	void draw(ID3D11DeviceContext* dc);

	std::function<void(ParticleUpdateData*)> updateStdFunc;
};

