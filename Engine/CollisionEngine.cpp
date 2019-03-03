#include "CollisionEngine.h"
#include "Model.h"

CollisionEngine::CollisionEngine()
{
}


CollisionEngine::~CollisionEngine()
{
}



void CollisionEngine::registerModel(Model *model, BoundingVolumeType bvt)
{
	_colliders.push_back(generateCollider(model, bvt));
	_models.push_back(model);
}



Collider CollisionEngine::generateCollider(Model* model, BoundingVolumeType bvt)
{
	std::vector<Hull*> hulls;
	hulls.reserve(model->meshes.size());

	switch (bvt)
	{
	case BVT_AABB:
		for (Mesh m : model->meshes) hulls.push_back(genBoxHull(&m));
		break;

	case BVT_SPHERE:
		for (Mesh m : model->meshes) hulls.push_back(genSphereHull(&m));
		break;
	}

	return Collider(bvt, model, hulls);
}



void CollisionEngine::unregisterModel(const Model* model)
{
	_models.erase(remove(_models.begin(), _models.end(), model), _models.end());
	
	for (Collider& c : _colliders)
	{
		if (c.parent == model) {
			_colliders.erase(remove(_colliders.begin(), _colliders.end(), c), _colliders.end());
			break;
		}
	}

}



void CollisionEngine::registerController(Controller* controller)
{
	_controller = controller;
}



void CollisionEngine::notifyController(const SVec3& resolution) const
{
	_controller->setCollisionOffset(resolution);
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
	aabb->min = SVec3(minX, minY, minZ);
	aabb->max = SVec3(maxX, maxY, maxZ);

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



//helper function(s) for intersection
inline float sq(float x) { return x * x; }

float Collider::SQD_PointAABB(SVec3 p, AABB b)
{
	float sqDist = 0.0f;
	float x = p.x,
		y = p.y,
		z = p.z;

	if (x < b.min.x) sqDist += sq(b.min.x - x);
	if (x > b.max.x) sqDist += sq(x - b.max.x);

	if (y < b.min.y) sqDist += sq(b.min.y - y);
	if (y > b.max.y) sqDist += sq(y - b.max.y);

	if (z < b.min.z) sqDist += sq(b.min.z - z);
	if (z > b.max.z) sqDist += sq(z - b.max.z);

	return sqDist;
}



///intersection tests
bool Collider::Collide(const Collider& other, SVec3& resolutionVector)
{
	bool collides = false;

	for (Hull* hull1 : hulls)
	{
		for (Hull* hull2 : other.hulls)
		{
			if(BVT == BVT_AABB)		collides = reinterpret_cast<AABB*>(hull1)->intersect(hull2, other.BVT);
			if (BVT == BVT_SPHERE)	collides = reinterpret_cast<SphereHull*>(hull1)->intersect(hull2, other.BVT);
			
			if (collides)
			{
				resolutionVector = hull2->getPosition() - hull1->getPosition();
				resolutionVector.Normalize();
				break;
			}
		}
	}

	return collides;
}



bool Collider::AABBSphereIntersection(const AABB& b, const SphereHull& s)
{
	return  SQD_PointAABB(s.c, b) <= sq(s.r);
}



bool Collider::SphereSphereIntersection(const SphereHull& s1, const SphereHull& s2)
{
	return SVec3::DistanceSquared(s1.c, s2.c) < sq(s1.r + s2.r);
}



bool Collider::AABBAABBIntersection(const AABB& a, const AABB& b)
{
	if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
	if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
	if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
	return true;
}


//not implemented @TODO
bool Collider::RaySphereIntersection(const SRay& ray, const SphereHull& s)
{
	return false;
}


//not implemented @TODO
bool Collider::RayAABBIntersection(const SRay& ray, const AABB& b)
{
	return false;
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



bool AABB::intersect(Hull* other, BoundingVolumeType otherType)
{
	if (otherType == BVT_SPHERE)		return Collider::AABBSphereIntersection(*this, *(reinterpret_cast<SphereHull*>(other)));
	else if (otherType == BVT_AABB)		return Collider::AABBAABBIntersection(*this, *(reinterpret_cast<AABB*>(other)));
	return false;
}



bool SphereHull::intersect(Hull* other, BoundingVolumeType otherType)
{
	if (otherType == BVT_SPHERE)		return Collider::SphereSphereIntersection(*this, *(reinterpret_cast<SphereHull*>(other)));
	else if (otherType == BVT_AABB)		return Collider::AABBSphereIntersection(*(reinterpret_cast<AABB*>(other)), *this);
	return false;
}



void Grid::populateCells(std::vector<Procedural::Terrain*>& terrain)
{

	CellKey cellKey;

	for (Procedural::Terrain* t : terrain)
	{
		for (Vert3D v : t->getVerts())
		{
			cellKey.assign( v.pos * invCellSize );

			if (cells.find(cellKey) == cells.end())
			{
				GridCell gc = GridCell(cellKey);
				gc.terrains.push_back(t);
				cells.insert(std::make_pair(cellKey, gc));
			}
			else
			{
				if (std::find(cells.at(cellKey).terrains.begin(), cells.at(cellKey).terrains.end(), t) == cells.at(cellKey).terrains.end())
					cells.at(cellKey).terrains.push_back(t);
			}
				
		}
	}
}



SVec3 CollisionEngine::adjustHeight(const SVec3& playerPos)
{
	
	CellKey cellKey;
	cellKey.assign(playerPos * grid.invCellSize);
	
	float finalHeight = 0.f;
	

	if (grid.cells.find(cellKey) == grid.cells.end())
		return playerPos;

	for (auto t : grid.cells.at(cellKey).terrains)
	{
		finalHeight = t->getHeightAtPosition(playerPos) + 10.f;
		return SVec3(playerPos.x, finalHeight, playerPos.z);
	}
}









/*
Hull* CollisionEngine::genQuickHull(Mesh* mesh)
{
	quickhull::QuickHull<float> qh;
	std::vector<quickhull::Vector3<float>> pointCloud;

	pointCloud.reserve(mesh->vertices.size());
	for (Vert3D v : mesh->vertices)
		pointCloud.push_back(quickhull::Vector3<float>(v.pos.x, v.pos.y, v.pos.z));

	auto hull = qh.getConvexHull(pointCloud, false, false);
	auto indexBuffer = hull.getIndexBuffer();
	auto vertexBuffer = hull.getVertexBuffer();

	CHull* ch = new CHull;
	ch->convexHull = hull;

	return ch;
}



template<typename FloatType>
FloatType quickhull::defaultEps()
{
	return 0.0001f;
}
*/