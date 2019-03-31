#pragma once
#include <vector>
#include "ParticleBase.h"
#include "ShaderBase.h"

class ParticleSystem
{
protected:


public:

	unsigned int _numParticles;
	Model _model;
	ShaderBase* _shader;
	ParticleUpdateData pud;
	SVec3 _position;
	std::vector<ParticleBase*> _particles;

	ParticleSystem();
	~ParticleSystem();

	void init(ID3D11Device* device, unsigned int particleCount, SVec3 position, std::string meshPath);
	void setShader(ShaderBase* shader);
	void setUpdateFunction(std::function<void(ParticleUpdateData* pud)> particleUpdFunc);
	void update(float deltaTime);
	void draw(ID3D11DeviceContext* dc);

	std::function<void(ParticleUpdateData*)> updateStdFunc;
};

