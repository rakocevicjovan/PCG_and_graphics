#pragma once

#include "SkeletalModel.h"
#include "SkeletalModelInstance.h"
#include "AnimationInstance.h"
#include "CSkModel.h"
#include "UpdateableBuffer.h"

//class AnimationInstance;

class SkAnimRender
{
public:

	void addInstance(entt::registry& registry, AnimationInstance& instance);


	void update(entt::registry& registry, float dTime);


	void render(entt::registry& registry);
};