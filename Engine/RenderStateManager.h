#pragma once
#include <d3d11.h>
#include <vector>
#include "RenderState.h"


//used to check which state entries need to be updated and which stay the same, stores the last used state
class RenderStateManager
{
public:

	RenderState _rs;

	//@TODO make this replace the majority of renderer's render
	void deltaUpdate()
	{
		//if state x not value y, update to value y and set current to new... for each state
	}

};