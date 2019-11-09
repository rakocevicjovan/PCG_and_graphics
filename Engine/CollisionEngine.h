#pragma once
#include <vector>

#include <list>
#include <d3d11.h>
#include "Controller.h"
#include "Grid.h"
#include "Hull.h"
#include "Collider.h"

class Actor;
class Model;
class Mesh;
class Collider;


class CollisionEngine
{
	Grid grid;

	Controller* _controller;
	std::list<Collider*> _colliders;

public:

	Hull* genSphereHull(Mesh* mesh, const SMatrix& transform, Collider* collider = nullptr);
	Hull* genBoxHull(Mesh* mesh, const SMatrix& transform, Collider* collider = nullptr);

	CollisionEngine();
	~CollisionEngine();

	void init() {}

	void registerModel(Model& model, BoundingVolumeType bvt);
	void registerActor(Actor& actor, BoundingVolumeType bvt);
	void unregisterModel(Model& model);
	void unregisterActor(Actor& actor);
	void addToGrid(Collider* collider);
	void removeFromGrid(Collider& collider);
	void update();

	void registerController(Controller& controller);
	HitResult resolvePlayerCollision(const SMatrix& playerTransform, SVec3& velocity);
};