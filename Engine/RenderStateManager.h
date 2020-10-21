#pragma once
#include <d3d11_4.h>
#include <vector>
#include "RenderState.h"


//used to check which state entries need to be updated and which stay the same, stores the gpu state and current scheduled state
class RenderStateManager
{
	RenderState _gpuState;
	RenderState _current;	//buffer zone

public:

	

	//@TODO make this replace the majority of renderer's render
	void deltaUpdate(RenderState next)
	{
		//simply copy everything to _current Render state, this needs to be more modular (functions for each state)
	}


	void applyToGPU()
	{
		//...set all the states to gpu state if not different, copy _current to _gpuState
	}

};