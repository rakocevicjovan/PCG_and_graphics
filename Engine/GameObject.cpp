#include "GameObject.h"
#include "Renderer.h"

Actor::Actor(Model* model, SMatrix& transform) : _steerComp(this), transform(transform)
{
	_collider = Collider(BoundingVolumeType::BVT_SPHERE, this, true);

	renderables.reserve(model->meshes.size());

	for (Mesh& mesh : model->meshes)
	{
		renderables.emplace_back(mesh);
		renderables.back().transform = mesh.transform;
		renderables.back().worldTransform = transform * mesh.transform;
		renderables.back().mat = &mesh._baseMaterial;

		_collider.addHull(new SphereHull(mesh.transform.Translation(), 1.f));		//@TODO see what to do about this
	}
}


void Actor::propagate()
{
	for (Renderable& r : renderables)
	{
		r.worldTransform = transform * r.transform;
	}

	//@TODO consider changing to per-mesh collider... could be cleaner tbh
	_collider.updateHullPositions();
}


void Actor::render(const Renderer& renderer) const
{
	for (const Renderable& r : renderables)
		renderer.render(r);
}