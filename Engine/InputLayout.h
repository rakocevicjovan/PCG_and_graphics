#pragma once
#include "Logger.h"
#include <d3d11_4.h>
#include <vector>


class InputLayout
{
	ID3D11InputLayout* _inLay;

	// NOT IMPLEMENTED YET!!!
	/*
	static bool CreateInputLayout(ID3D11Device* device, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inDesc, ID3D11InputLayout*& layout)
	{
		// Needs a shader blob so uh... make one... it's not elegant but it's possible
		// to generate one on the fly, perf isn't too important as this shouldn't happen 
		// during game runtime, likely only during asset precompilation

		
		// Create the layout based on the vertex shader, which is based on the layout... omegalul
		if (FAILED(_device->CreateInputLayout(inDesc.data(), inDesc.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &layout)))
		{
			OutputDebugStringA("Failed to create vertex input layout.");
			return false;
		}
		return true;
	}
	*/

};