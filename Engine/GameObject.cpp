#include "GameObject.h"
#include "Collider.h"

Actor::Actor(SMatrix& transform, Model* model) : transform(transform), _steerComp(this)
{
	collider = new Collider(BoundingVolumeType::BVT_SPHERE, this, true);

	renderables.reserve(model->meshes.size());

	for (Mesh& mesh : model->meshes)
	{
		renderables.emplace_back(mesh);
		renderables.back().transform = mesh.transform;
		renderables.back().worldTransform = transform * mesh.transform;
		renderables.back().mat = mesh.baseMaterial;

		collider->hulls.push_back(mesh.getHull());
		collider->hulls.back()->_collider = collider;	//wonderful way to put it innit?
	}
}