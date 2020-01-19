#pragma once
#include <d3d11.h>
#include <string>
#include <vector>

//more of a reference than for real use, keeps track of what is needed to describe shaders
class ShaderMeta
{
	std::string _filePath;
	std::vector<D3D11_INPUT_ELEMENT_DESC> _layoutDesc;
	std::vector<D3D11_BUFFER_DESC> _bufferDescs;
	const D3D11_SAMPLER_DESC& samplerDesc;
};