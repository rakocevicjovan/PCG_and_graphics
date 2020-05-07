#include "CollisionEngine.h"
#include "Math.h"
#include "GameObject.h"
#include "SimpleMath.h"	//nani?

/*
	SCREAM(bloodyMurder) std::ostringstream ss;	\
	ss << bloodyMurder << "\n"; \
	std::string s(ss.str()); \
	OutputDebugStringA(ss.str().c_str());
*/

CollisionEngine::CollisionEngine()
{
}


CollisionEngine::~CollisionEngine()
{
}



void CollisionEngine::registerActor(Actor& actor, BoundingVolumeType bvt)
{
}



void CollisionEngine::unregisterActor(Actor& actor)
{
	for (Collider* c : _colliders)
	{
		/*if (c == &actor._collider)
		{
			removeFromGrid(*c);
			c->ReleaseMemory();
			delete c;
			c = nullptr;
		}*/
	}

	//_colliders.erase(remove(_colliders.begin(), _colliders.end(), actor._collider), _colliders.end());
}



void CollisionEngine::addToGrid(Collider* collider)
{
	auto bvt = collider->BVT;

	if (bvt == BVT_AABB)
		for (Hull* h : collider->getHulls())
			grid.addAABB(reinterpret_cast<AABB*>(h));
			

	if (bvt == BVT_SPHERE)
		for (Hull* h : collider->getHulls())
			grid.addSphere(reinterpret_cast<SphereHull*>(h));
}



void CollisionEngine::removeFromGrid(Collider& collider)
{
	for (Hull* h : collider.getHulls())
	{
		for (auto& keyCellPair : grid.cells)
		{
			//keyCellPair.second.hulls.erase(std::remove(keyCellPair.second.hulls.begin(), keyCellPair.second.hulls.end(), h), keyCellPair.second.hulls.end());
			keyCellPair.second._hulls.erase(h);
		}
	}
}



void CollisionEngine::update()
{
	
	for (Collider* collider : _colliders)
	{
		if (collider->dynamic)
		{
			for (Hull* h : collider->getHulls())	//@TODO allow offsets for multiple hulls!
				h->setPosition(collider->_parent->transform.Translation());

			addToGrid(collider);
		}
	}
	

	for (auto iterator = grid.cells.begin(); iterator != grid.cells.end();)
	{
		if (iterator->second._hulls.empty())
			iterator = grid.cells.erase(iterator);
		else
			iterator++;
	}
}



void CollisionEngine::registerController(Controller& controller)
{
	_controller = &controller;
	controller._colEng = this;
}



HitResult CollisionEngine::resolvePlayerCollision(const SMatrix& playerTransform, SVec3& velocity)
{
	SphereHull playerHull;
	playerHull.ctr = playerTransform.Translation();
	playerHull.r = 5.f;

	CellKey ck(playerTransform.Translation(), grid.invCellSize), adjCK;

	SVec3 collisionNormal;
	std::vector<Hull*> collidedHulls;

	float maxPenDepth = -1.f;

	HitResult hitRes;

	for (int i = -1; i < 2; ++i)
	{
		adjCK.x = ck.x + i;

		for (int j = -1; j < 2; ++j)
		{
			adjCK.y = ck.y + j;

			for (int k = -1; k < 2; ++k)
			{
				adjCK.z = ck.z + k;

				for (auto hull : grid.cells[adjCK]._hulls)
				{
					HitResult hr = hull->intersect(&playerHull, BVT_SPHERE);
					if (hr.hit)
					{
						collidedHulls.push_back(hull);

						if (velocity.Dot(hr.resolutionVector) < 0.001f) 
							velocity -= Math::projectVecOntoVec(velocity, hr.resolutionVector);

						float curPenDepth = sqrt(hr.sqPenetrationDepth);
						collisionNormal += hr.resolutionVector * curPenDepth;

						if (curPenDepth > maxPenDepth)
						{
							hitRes = hr;
							maxPenDepth = hr.sqPenetrationDepth;
						}

						hr.resolutionVector = collisionNormal;

					}
				}


			}
		}
	}


	return hitRes;
}



Hull* CollisionEngine::genBoxHull(Mesh* mesh, const SMatrix& transform, Collider* c)
{
	float minX, minY, minZ, maxX, maxY, maxZ;
	auto pos = mesh->_vertices[0].pos;

	minX = maxX = pos.x;
	minY = maxY = pos.y;
	minZ = maxZ = pos.z;

	for (const Vert3D& vert : mesh->_vertices) 
	{
		SVec3 pos = SVec3::Transform(vert.pos, transform);

		minX = min(pos.x, minX);
		maxX = max(pos.x, maxX);

		minY = min(pos.y, minY);
		maxY = max(pos.y, maxY);

		minZ = min(pos.z, minZ);
		maxZ = max(pos.z, maxZ);
	}

	AABB* aabb = new AABB;
	aabb->minPoint = SVec3(minX, minY, minZ);
	aabb->maxPoint = SVec3(maxX, maxY, maxZ);

	aabb->_collider = c;

	return aabb;
}



Hull* CollisionEngine::genSphereHull(Mesh* mesh, const SMatrix& transform, Collider* collider)
{
	SVec3 center;
	float radius = 0.f;
	for (const Vert3D& v : mesh->_vertices)
	{
		center += v.pos;
		radius = max(radius, v.pos.LengthSquared());
	}

	center /= (float)(mesh->_vertices.size());
	radius = sqrt(radius);

	SphereHull* sh = new SphereHull;
	sh->ctr = center;
	sh->r = radius;

	return sh;
}