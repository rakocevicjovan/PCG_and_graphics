#include "GameObject.h"
#include "Renderer.h"



Actor::Actor(Model* model, SMatrix& transform) 
	: _steerComp(this), _transform(transform), _collider(Collider(BoundingVolumeType::BVT_SPHERE, this, true))
{
	_renderables.reserve(model->_meshes.size());

	for (Mesh& mesh : model->_meshes)
	{
		_renderables.emplace_back(mesh);
		_renderables.back()._localTransform = mesh._transform;
		_renderables.back()._transform = transform * mesh._transform;
		_renderables.back().mat = &mesh._baseMaterial;

		_collider.addHull(new SphereHull(_renderables.back()._transform.Translation(), 1.f * transform._11));		//@TODO see what to do about this
	}
}



Actor::Actor(const Actor& other) 
	: _steerComp(other._steerComp), _collider(Collider(other._collider.BVT, this, other._collider.dynamic))
{
	_transform = other._transform;

	for (Hull* sp : other._collider.getHulls())
		_collider.addHull(new SphereHull(sp->getPosition(), sp->getExtent()));

	_renderables.reserve(other._renderables.size());
	for (const Renderable& r : other._renderables)
		_renderables.push_back(r);
}



void Actor::patchMaterial(VertexShader* vs, PixelShader* ps, PointLight& pLight)
{
	for (Renderable& r : _renderables)
	{
		r.mat->setVS(vs);
		r.mat->setPS(ps);
	}
}



void Actor::propagate()
{
	for (Renderable& r : _renderables)
	{
		r._transform = _transform * r._localTransform;
	}

	//@TODO consider changing to per-mesh collider... could be cleaner tbh
	_collider.updateHullPositions();
}



void Actor::render(const Renderer& renderer) const
{
	for (const Renderable& r : _renderables)
		renderer.render(r);
}



void Actor::addToRenderQueue(Renderer& renderer, const SVec3& camPos, const SVec3& viewForward)
{
	for (Renderable& r : _renderables)
	{
		r.zDepth = (_transform.Translation() - camPos).Dot(viewForward);
		renderer.addToRenderQueue(r);
	}
}



void Actor::addRenderable(const Renderable& renderable, float r)
{
	_renderables.push_back(renderable);
	_collider.addHull(new SphereHull(renderable._localTransform.Translation(), r));
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