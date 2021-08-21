#pragma once

#include "SkeletalModel.h"
#include "SkeletalModelInstance.h"
#include "AnimationInstance.h"
#include "CSkModel.h"
#include "UpdateableBuffer.h"

class SkAnimRender
{
public:

	void update(entt::registry& registry, float dTime)
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


	void render(entt::registry& registry)
	{

	}
};