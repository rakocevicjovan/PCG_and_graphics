#include "GameObject.h"
#include "Collider.h"

Actor::Actor(SMatrix& transform, Model* model) : transform(transform), _steerComp(this)
{
	_collider = new Collider(BoundingVolumeType::BVT_SPHERE, this, true);

	renderables.reserve(model->meshes.size());

	for (Mesh& mesh : model->meshes)
	{
		renderables.emplace_back(mesh);
		renderables.back().transform = mesh.transform;
		renderables.back().worldTransform = transform * mesh.transform;
		renderables.back().mat = &mesh._baseMaterial;

		_collider->hulls.push_back(new SphereHull(mesh.transform.Translation(), 1.f));	//@TODO see what to do about this
		_collider->hulls.back()->_collider = _collider;	//wonderful way to put it innit?
	}
}


void Actor::propagate()
{
	for (Renderable& r : renderables)
	{
		r.worldTransform = transform * r.transform;
	}

	//@TODO consider changing to per-mesh collider... could be cleaner tbh
	_collider->updateHullPositions();
}