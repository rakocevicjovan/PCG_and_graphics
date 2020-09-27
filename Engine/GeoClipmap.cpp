#include "GeoClipmap.h"
#include "VertSignature.h"
#include "Shader.h"


GeoClipmap::GeoClipmap(UINT numLayers, UINT edgeSizeLog2)
	: _numLayers(numLayers), _w(pow(2, edgeSizeLog2) - 1)
{
	_x = (_w + 1) / 4;
	_gapSize = _x * 3;	// Cardinal gaps
}



void GeoClipmap::init(ID3D11Device* device)
{
	std::vector<SVec2> vertXYs;
	vertXYs.reserve(_x * _x);

	// @TODO replace with bufferless, no need for it on newer GPUs
	for (int i = 0; i < _x; ++i)
	{
		for (int j = 0; j < _x; ++j)
		{
			vertXYs.emplace_back(j, i);	// Apply some scaling here, not sure how to determine it.
		}
	}

	VBuffer centerBuffer(device, nullptr, 0, sizeof(SVec2));

	VBuffer blockBuffer(device, vertXYs.data(), vertXYs.size() * sizeof(SVec2), sizeof(SVec2));
	
	ID3D11InputLayout* inputLayout;
	VertSignature vertSig;
	vertSig.addAttribute(VAttribSemantic::POS, VAttribType::FLOAT2, 1u, 4);
	std::vector<D3D11_INPUT_ELEMENT_DESC> ied = vertSig.createVertInLayElements();

	// Instantiate the vertex shader as well
}



void GeoClipmap::draw()
{

}