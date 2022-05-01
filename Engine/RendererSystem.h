#pragma once

#include "Camera.h"
#include "Math.h"
#include "ColFuncs.h"

#include "entt/entt.hpp"

struct VisibleFlag
{
	uint32_t val;
};

class RendererSystem
{
private:

	// SIMD assumes even number of OBBs, so make a fake one belonging to an otherwise inaccessible entity in case it's required
	// This should be called lazily by frustum cull with the intention of not doing it on insert/remove but only when it's needed
	entt::entity _extra{ entt::null };
	bool _has_extra{ true };

	void assure_valid()
	{
		auto num_spheres = _registry->view<SphereHull>().size();
		bool odd = num_spheres & 1;

		// If the number of hulls is even, do nothing
		if (odd)
		{
			// If it's odd, either add or remove one set of components to make sure it's even
			if (_has_extra)
			{
				_registry->remove<SphereHull>(_extra);
				_registry->remove<VisibleFlag>(_extra);
				_has_extra = false;
			}
			else
			{
				_registry->emplace<SphereHull>(_extra, SphereHull{});
				_registry->emplace<VisibleFlag>(_extra, VisibleFlag{ 0u });
				_has_extra = true;
			}
		}
	};

public:

	entt::registry* _registry;

	RendererSystem(entt::registry* registry) : _registry(registry)
	{
		_extra = _registry->create();
		_registry->emplace<SphereHull>(_extra, SphereHull{});
		_registry->emplace<VisibleFlag>(_extra, VisibleFlag{ 0u });
	}

	void frustumCull(const Camera& cam);

	void addHull(entt::entity entity, const SphereHull& hull)
	{
		_registry->emplace<SphereHull>(entity, hull);
	}

	void removeHull(entt::entity entity)
	{
		_registry->remove<SphereHull>(entity);
	}
};