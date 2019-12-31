#pragma once
#include <vector>
#include "Math.h"
#include "Systems.h"
#include "GameObject.h"

struct Lilly
{
	Lilly(SMatrix transform, UINT ring) : ring(ring)
	{
		act.transform = transform;
	}

	Actor act;
	UINT ring;
	bool real = false;
};



struct Ring
{
	Ring(float dist) : _distance(dist) {};

	float _distance;
	std::vector<Lilly> _lillies;
	float rotSpeed;
};



class Lillies
{
	float _lillyRadius, _ringThickness, _rotationRate;
	UINT _numRings = 0u, _numLillies = 0u;
	SVec2 _edges;
	SVec3 _midPoint;

	std::vector<InstanceData> instanceData;
	std::vector<InstanceData> realData;

public:

	Lillies();
	~Lillies();

	std::vector<Ring> _lillyRings;

	void init(float lillyRadius, float rotationRate, SVec2 edges, SVec3 center);
	void luckOfTheDraw();
	void update(float dTime);
	void draw(const RenderContext& rc, Model& lillyModel, const PointLight& pLight, bool isFiltering);
};