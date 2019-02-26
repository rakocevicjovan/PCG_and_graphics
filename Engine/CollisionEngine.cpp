#include "CollisionEngine.h"


CollisionEngine::CollisionEngine()
{
}


CollisionEngine::~CollisionEngine()
{
}



void CollisionEngine::registerModel(Model * model, BoundingVolumeType bvt)
{
	generateHulls(model, bvt);
	_models.push_back(model);
}



Collider CollisionEngine::generateHulls(Model* model, BoundingVolumeType bvt)
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

	/*case BVT_QUICKHULL:
		for (Mesh m : model->meshes) hulls.push_back(genQuickHull(&m));
		break;*/
	}

	return Collider(bvt, model, hulls);
}



void CollisionEngine::unregisterModel(const Model* model)
{
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