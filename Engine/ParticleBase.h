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



struct ParticleBase
{
	ParticleSystem* _ps;

	SVec3 velocity;
	SVec3 acceleration;

	SMatrix transform;

	float age;
};



struct Lantern : public ParticleBase
{
	float luminosity;
	SVec4 colour;
};