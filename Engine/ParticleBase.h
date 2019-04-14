#pragma once
#include <d3d11.h>
#include "Math.h"

class ParticleSystem;

struct ParticleUpdateData {};



struct WindUpdateData : ParticleUpdateData
{
	SVec3 windDirection;
	float windVelocity;
	float dTime;
};



struct DragonUpdateData : ParticleUpdateData
{
	SVec3 playerPos;
	SVec3 wind;
	float speed;
	float dTime;
};

struct ParticleBase
{
	ParticleSystem* _ps;

	SVec3 velocity;
	SVec3 acceleration;

	SMatrix transform;

	float age;

	void update(float deltaTime);
	void draw(ID3D11DeviceContext* dc);
};



struct Lantern : public ParticleBase
{
	float luminosity;
	SVec4 colour;
};