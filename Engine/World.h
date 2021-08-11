#pragma once

#include "entt/entt.hpp"
#include "CTransform.h"
#include "CModel.h"
#include "CSkModel.h"

// Data describing things placed in the game world, can be loaded as a "level" or streamed to form a seamless open world... at least that's the idea


class World
{
public:

	entt::registry _reg;

	World()
	{
		//entt::group<CModel, CTransform> wat = 
		//_reg.group<CModel, CTransform>();
		//_reg.group<CSkModel, CTransform>();
	}
};