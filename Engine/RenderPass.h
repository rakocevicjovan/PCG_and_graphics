#pragma once
#include "RenderTarget.h"



class RenderPass
{
public:

	RenderPass(ID3D11Device* device)
	{
		
	}

	void bind(ID3D11DeviceContext* context)
	{
		_renderTarget.bind(context);
	}

private:

	RenderTarget _renderTarget;
};