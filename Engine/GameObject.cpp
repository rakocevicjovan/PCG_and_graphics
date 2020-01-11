#include "GameObject.h"
#include "Renderer.h"



Actor::Actor(Model* model, SMatrix& transform) 
	: _steerComp(this), transform(transform), _collider(Collider(BoundingVolumeType::BVT_SPHERE, this, true))
{
	renderables.reserve(model->meshes.size());

	for (Mesh& mesh : model->meshes)
	{
		renderables.emplace_back(mesh);
		renderables.back().transform = mesh.transform;
		renderables.back().worldTransform = transform * mesh.transform;
		renderables.back().mat = &mesh._baseMaterial;

		_collider.addHull(new SphereHull(mesh.transform.Translation(), 1.f * transform._11));		//@TODO see what to do about this
	}
}


void Actor::patchMaterial(VertexShader* vs, PixelShader* ps, PointLight& pLight)
{
	for (Renderable& r : renderables)
	{
		r.mat->setVS(vs);
		r.mat->setPS(ps);
		r.pLight = &pLight;		//this is awkward and I don't know how to do it properly right now...
	}
}


Actor::Actor(const Actor& other) 
	: _steerComp(other._steerComp), _collider(Collider(other._collider.BVT, this, other._collider.dynamic))
{
	transform = other.transform;

	for (Hull* sp : other._collider.getHulls())
		_collider.addHull(new SphereHull(sp->getPosition(), sp->getExtent()));

	renderables.reserve(other.renderables.size());
	for (const Renderable& r : other.renderables)
		renderables.push_back(r);
}



Actor::~Actor() {}


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


/*
void Actor::copyShenanigans(const Actor& other)
{
	_collider = other._collider;
	_collider.clearHullsNoDelete();

	for (Hull* sp : other._collider.getHulls())
		_collider.addHull(new SphereHull(sp->getPosition(), sp->getExtent()));

	renderables.reserve(other.renderables.size());
	for (const Renderable& r : other.renderables)
		renderables.push_back(r);
}
*/