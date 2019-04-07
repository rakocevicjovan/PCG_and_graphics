#include "CollisionEngine.h"
#include "Model.h"
#include "Math.h"
#include "GameObject.h"


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


//tbd
void CollisionEngine::registerModel(Model& model, BoundingVolumeType bvt)
{
	_colliders.push_back(new Collider());
	_colliders.back()->modParent = &model;
	_colliders.back()->BVT = bvt;
	_colliders.back()->dynamic = false;

	model.collider = _colliders.back();

	switch (bvt)
	{
	case BVT_AABB:
		for (Mesh m : model.meshes) _colliders.back()->hulls.push_back(genBoxHull(&m));
		break;

	case BVT_SPHERE:
		for (Mesh m : model.meshes) _colliders.back()->hulls.push_back(genSphereHull(&m));
		break;
	}

	addToGrid(_colliders.back());
}



void CollisionEngine::registerActor(Actor& actor, BoundingVolumeType bvt)
{
	_colliders.push_back(new Collider());
	_colliders.back()->actParent = &actor;
	_colliders.back()->BVT = bvt;
	_colliders.back()->dynamic = true;

	actor.collider = _colliders.back();

	switch (bvt)
	{
	case BVT_AABB:
		for (Mesh m : actor.gc.model->meshes) _colliders.back()->hulls.push_back(genBoxHull(&m));
		break;

	case BVT_SPHERE:
		for (Mesh m : actor.gc.model->meshes) _colliders.back()->hulls.push_back(genSphereHull(&m));
		break;
	}

	addToGrid(_colliders.back());
}



void CollisionEngine::unregisterModel(Model& model)
{
	for (Collider* c : _colliders)
	{
		if (c->modParent == &model)
		{
			removeFromGrid(*c);
			c->ReleaseMemory();
			delete c;
			c = nullptr;
		}
	}
	_colliders.erase(remove(_colliders.begin(), _colliders.end(), model.collider), _colliders.end());
}



void CollisionEngine::unregisterActor(Actor& actor)
{

	for (Collider* c : _colliders)
	{
		if (c == actor.collider)
		{
			removeFromGrid(*c);
			c->ReleaseMemory();
			delete c;
			c = nullptr;
		}
	}

	_colliders.erase(remove(_colliders.begin(), _colliders.end(), actor.collider), _colliders.end());
}



void CollisionEngine::addToGrid(Collider* collider)
{
	auto bvt = collider->BVT;

	if (bvt == BVT_AABB)
		for (Hull* h : collider->hulls)
			grid.addAABB(reinterpret_cast<AABB*>(h));
			

	if (bvt == BVT_SPHERE)
		for (Hull* h : collider->hulls)
			grid.addSphere(reinterpret_cast<SphereHull*>(h));
}



void CollisionEngine::removeFromGrid(Collider& collider)
{
	for (Hull* h : collider.hulls)
	{
		for (auto& keyCellPair : grid.cells)
		{
			//keyCellPair.second.hulls.erase(std::remove(keyCellPair.second.hulls.begin(), keyCellPair.second.hulls.end(), h), keyCellPair.second.hulls.end());
			keyCellPair.second.hulls.erase(h);
		}
	}
}



void CollisionEngine::update()
{
	/*
	for (Collider* collider : _colliders)
	{
		if (collider->dynamic)
		{
			SVec4 pos = collider->actParent->transform.Translation();
			SVec3 pos3 = SVec3(pos.x, pos.y, pos.z);
			collider->transform = collider->actParent->transform; 

			for (Hull* h : collider->hulls) h->setPosition(pos3);

			addToGrid(collider);
		}
	}
	*/

	for (auto iterator = grid.cells.begin(); iterator != grid.cells.end();)
	{
		if (iterator->second.hulls.empty())
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



SVec3 CollisionEngine::resolvePlayerCollision(const SMatrix& playerTransform, SVec3& velocity)
{
	SphereHull playerHull;
	playerHull.c = playerTransform.Translation();
	playerHull.r = 5.f;

	CellKey ck(playerTransform.Translation(), grid.invCellSize), adjCK;

	SVec3 collisionNormal;
	std::vector<Hull*> collidedHulls;

	for (int i = -1; i < 2; ++i)
	{
		adjCK.x = ck.x + i;

		for (int j = -1; j < 2; ++j)
		{
			adjCK.y = ck.y + j;

			for (int k = -1; k < 2; ++k)
			{
				adjCK.z = ck.z + k;

				for (auto hull : grid.cells[adjCK].hulls)
				{
					HitResult hr = hull->intersect(&playerHull, BVT_SPHERE);
					if (hr.hit)
					{
						collidedHulls.push_back(hull);

						if (velocity.Dot(hr.resolutionVector) < 0.001f) 
							velocity -= Math::projectVecOntoVec(velocity, hr.resolutionVector);

						collisionNormal += hr.resolutionVector * sqrt(hr.sqPenetrationDepth);
					}
				}


			}
		}
	}

	return collisionNormal;
}



Hull* CollisionEngine::genBoxHull(Mesh* mesh)
{
	float minX, minY, minZ, maxX, maxY, maxZ;
	auto pos = mesh->vertices[0].pos;

	minX = maxX = pos.x;
	minY = maxY = pos.y;
	minZ = maxZ = pos.z;

	for (const Vert3D& v : mesh->vertices) 
	{
		minX = min(v.pos.x, minX);
		maxX = max(v.pos.x, maxX);

		minY = min(v.pos.y, minY);
		maxY = max(v.pos.y, maxY);

		minZ = min(v.pos.z, minZ);
		maxZ = max(v.pos.z, maxZ);
	}

	AABB* aabb = new AABB;
	aabb->minPoint = SVec3(minX, minY, minZ);
	aabb->maxPoint = SVec3(maxX, maxY, maxZ);

	return aabb;
}



Hull* CollisionEngine::genSphereHull(Mesh* mesh)
{
	SVec3 center;
	float radius = 0.f;
	for (const Vert3D& v : mesh->vertices)
	{
		center += v.pos;
		radius = max(radius, v.pos.LengthSquared());
	}

	center /= (float)(mesh->vertices.size());
	radius = sqrt(radius);

	SphereHull* sh = new SphereHull;
	sh->c = center;
	sh->r = radius;

	return sh;
}



///intersection tests
HitResult Collider::Collide(const Collider& other, SVec3& resolutionVector)
{
	HitResult hitRes;

	for (Hull* hull1 : hulls)
	{
		for (Hull* hull2 : other.hulls)
		{
			if(BVT == BVT_AABB)		hitRes = reinterpret_cast<AABB*>(hull1)->intersect(hull2, other.BVT);
			if (BVT == BVT_SPHERE)	hitRes = reinterpret_cast<SphereHull*>(hull1)->intersect(hull2, other.BVT);
			
			if (hitRes.hit)
			{
				resolutionVector = hull2->getPosition() - hull1->getPosition();
				resolutionVector.Normalize();
				break;
			}
		}
	}

	return hitRes;
}



//ACTUAL INTERSECTION CODE AFTER ALL THIS PAPERWORK
inline float sq(float x) { return x * x; }

float Collider::ClosestPointOnAABB(SVec3 p, AABB b, SVec3& out)
{
	out.x = Math::clamp(b.minPoint.x, b.maxPoint.x, p.x);
	out.y = Math::clamp(b.minPoint.y, b.maxPoint.y, p.y);
	out.z = Math::clamp(b.minPoint.z, b.maxPoint.z, p.z);

	return SVec3::DistanceSquared(p, out);
}


//resolution vector is the vector from the sphere center to the closest point
HitResult Collider::AABBSphereIntersection(const AABB& b, const SphereHull& s)
{
	HitResult hr;
	SVec3 closestPointOnAABB;

	float sqdToClosestPoint = ClosestPointOnAABB(s.c, b, closestPointOnAABB);
	float sqpenetrationDepth = sq(s.r) - sqdToClosestPoint;

	hr.hit = sqpenetrationDepth > 0;
	hr.sqPenetrationDepth = hr.hit ? sqpenetrationDepth : 0.f;

	SVec3 resVec = s.c - closestPointOnAABB ;	//if sphere is in the object this will be 0...

	if (resVec.LengthSquared() < 0.0001f)
		hr.resolutionVector = Math::getNormalizedVec3(closestPointOnAABB - b.getPosition());
	else
		hr.resolutionVector = Math::getNormalizedVec3(resVec);

	return hr;
}



HitResult Collider::SphereSphereIntersection(const SphereHull& s1, const SphereHull& s2)
{
	float distSquared = SVec3::DistanceSquared(s1.c, s2.c);
	return HitResult(distSquared < sq(s1.r + s2.r), s1.c - s2.c, distSquared);
}



HitResult Collider::AABBAABBIntersection(const AABB& a, const AABB& b)
{
	if (a.maxPoint.x < b.minPoint.x || a.minPoint.x > b.maxPoint.x) return HitResult();
	if (a.maxPoint.y < b.minPoint.y || a.minPoint.y > b.maxPoint.y) return HitResult();
	if (a.maxPoint.z < b.minPoint.z || a.minPoint.z > b.maxPoint.z) return HitResult();
	return HitResult(true, a.getPosition() - b.getPosition(), 0.f);
}


//not implemented @TODO
bool Collider::RaySphereIntersection(const SRay& ray, const SphereHull& s)
{
	return false;
}



bool ClipLine(int dim, const AABB& b, const SRay& lineSeg, float& lo, float& hi)
{
	float fDimLow, fDimHigh;
	float inv = 1.f / (lineSeg.direction.at(dim) - lineSeg.position.at(dim));

	fDimLow = (b.minPoint.at(dim) - lineSeg.position.at(dim)) * inv;
	fDimHigh = (b.maxPoint.at(dim) - lineSeg.position.at(dim)) * inv;

	if (fDimHigh < fDimLow)
		Math::swap(fDimHigh, fDimLow);

	if (fDimHigh < lo) return false;
	if (fDimLow > hi) return false;

	lo = max(fDimLow, lo);
	hi = max(fDimHigh, hi);

	return (lo < hi);
}



//ray structure used for convenience, it's actually first and last point of the line
HitResult Collider::RayAABBIntersection(const SRay& lineSeg, const AABB& b, SVec3& poi, float& t)
{
	HitResult hr;

	SVec3 lineDirection = lineSeg.direction - lineSeg.position;

	float invDeltaX = 1.f / (lineDirection.x);
	float invDeltaY = 1.f / (lineDirection.y);
	float invDeltaZ = 1.f / (lineDirection.z);

	float flow = 0.f;
	float fhigh = 1.f;

	for (int i = 0; i < 3; ++i)
	{
		hr.hit = ClipLine(i, b, lineSeg, flow, fhigh);
		if (!hr.hit) return hr;
	}

	poi = lineSeg.position + lineDirection * flow;
	t = flow;

	return hr;
}



bool Collider::RayPlaneIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c, SVec3& intersectionPoint)
{
	SVec3 normal = (b - a).Cross(c - b);

	float t = (c - ray.position).Dot(normal) / ray.direction.Dot(normal);

	if (t > 0 && t < 1)
	{
		intersectionPoint = ray.position + t * ray.direction;
		return true;
	}
	return false;
}



bool Collider::RayTriangleIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c)
{
	SVec3 i;
	if (!RayPlaneIntersection(ray, a, b, c, i)) return false;

	SVec3 ab = b - a;
	SVec3 cb = b - c;
	SVec3 ac = c - a;

	SVec3 projABOntoCB = Math::projectVecOntoVec(ab, cb);
	SVec3 v = ab - projABOntoCB;

	SVec3 ai = i - a;

	float aBar = 1 - (v.Dot(ai) / v.Dot(ab));

	if (aBar < 0.f || aBar > 1.f)
		return false;

	SVec3 projABontoAC = Math::projectVecOntoVec(ab, ac);
	v = -ab + projABontoAC;
	SVec3 bi = i - b;

	float bBar = 1 - (v.Dot(bi) / v.Dot(-ab));
		
	return bBar > 0.f && bBar < 1.f;
}



HitResult AABB::intersect(const Hull* other, BoundingVolumeType otherType) const
{
	if (otherType == BVT_SPHERE)		return Collider::AABBSphereIntersection(*this, *(reinterpret_cast<const SphereHull*>(other)));
	else if (otherType == BVT_AABB)		return Collider::AABBAABBIntersection(*this, *(reinterpret_cast<const AABB*>(other)));
	return HitResult();
}



HitResult SphereHull::intersect(const Hull* other, BoundingVolumeType otherType) const
{
	if (otherType == BVT_SPHERE)		return Collider::SphereSphereIntersection(*this, *(reinterpret_cast<const SphereHull*>(other)));
	else if (otherType == BVT_AABB)		return Collider::AABBSphereIntersection(*(reinterpret_cast<const AABB*>(other)), *this);
	return HitResult();
}



void Grid::addAABB(AABB* h)
{
	std::vector<SVec3> positions = h->getVertices();

	CellKey minCellKey(h->minPoint, invCellSize);
	CellKey maxCellKey(h->maxPoint, invCellSize);

	for (int i = minCellKey.x; i <= maxCellKey.x; ++i)
		for (int j = minCellKey.y; j <= maxCellKey.y; ++j)
			for (int k = minCellKey.z; k <= maxCellKey.z; ++k)
				cells[CellKey(i, j, k)].hulls.insert(h);
}


//@TODO...
void Grid::addSphere(SphereHull* h)
{
	
}





std::vector<SVec3> AABB::getVertices() const
{
	return
	{
		minPoint,						//left  lower  near
		SVec3(minPoint.x, minPoint.y, maxPoint.z),	//left  lower  far
		SVec3(maxPoint.x, minPoint.y, minPoint.z),	//right lower  near
		SVec3(maxPoint.x, minPoint.y, maxPoint.z),	//right lower  far
		maxPoint,						//right higher far
		SVec3(minPoint.x, maxPoint.y, maxPoint.z),	//left  higher far
		SVec3(maxPoint.x, maxPoint.y, minPoint.z),	//right higher near
		SVec3(minPoint.x, maxPoint.y, minPoint.z),	//left  higher near
	};
}


//b t n f l r
std::vector<SPlane> AABB::getPlanes() const
{
	
	std::vector<SVec3> v = getVertices();
	std::vector<SPlane> result;
	result.reserve(6);

	result.emplace_back(v[0], v[1], v[2]);	//bottom
	result.emplace_back(v[4], v[5], v[6]);	//top
	result.emplace_back(v[0], v[2], v[6]);	//near
	result.emplace_back(v[1], v[3], v[4]);	//far
	result.emplace_back(v[0], v[1], v[5]);	//left
	result.emplace_back(v[2], v[3], v[4]);	//right

	return result;
}