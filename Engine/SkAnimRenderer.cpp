#include "pch.h"

#include "SkAnimRenderer.h"


void SkAnimRender::addInstance(entt::registry& registry, AnimationInstance& instance)
{

}


void SkAnimRender::update(entt::registry& registry, float dTime)
{
	auto animationInstances = registry.view<AnimationInstance>();

	animationInstances.each(
		[&dTime](AnimationInstance& animInstance)
		{
			animInstance.update(dTime);

			/*
			SkeletalModelInstance skModelInst;
			skModelInst.update(dTime, animInstance);
			Math::SetTranslation(skModelInst._transform, offset);
			Math::SetScale(skModelInst->_transform, SVec3(_previewScale));
			skModelInst.draw(context);
			*/
		});
}


// Assume no culling for now
void SkAnimRender::render(entt::registry& registry)
{

}