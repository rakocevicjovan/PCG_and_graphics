#include "GeoClipmap.h"
#include "VertSignature.h"
#include "Shader.h"



GeoClipmap::GeoClipmap(UINT numLayers, UINT edgeSizeLog2, float xzScale)
	: _numLayers(numLayers), _edgeVertCount(pow(2, edgeSizeLog2) - 1), _coreVertSpacing(xzScale)
{
	_blockEdgeVertCount = (_edgeVertCount + 1) / 4;	// Size of the outer layer block in vertices
	_gapSize = _blockEdgeVertCount * 3;	// Cardinal gaps
	_layers.resize(_numLayers);

	// Texture size
	_texSize[0] = _texSize[1] = (_edgeVertCount + 1) + 2 * _numLayers * _blockEdgeVertCount;
}



void GeoClipmap::init(ID3D11Device* device)
{
	// Vertex buffers - consider doing this in morton order?
	// Central patch buffer

	std::vector<SVec2> coreVertices;
	for (int i = 0; i < _edgeVertCount; ++i)
	{
		for (int j = 0; j < _edgeVertCount; ++j)
			coreVertices.emplace_back(j, i);
	}

	VBuffer centerBuffer(device, coreVertices.data(), coreVertices.size() * sizeof(SVec2), sizeof(SVec2));



	// Vertex shader
	VertSignature vertSig;
	vertSig.addAttribute(VAttribSemantic::POS, VAttribType::FLOAT2, 1u, 4u);	// Will be backed into float16
	std::vector<D3D11_INPUT_ELEMENT_DESC> inLayDesc = vertSig.createVertInLayElements();

	D3D11_BUFFER_DESC bufferDesc = CBuffer::createDesc(sizeof(GeoClipmapBuffer));

	VertexShader vs(device, L"GeoClipmapVS.hlsl", inLayDesc, { bufferDesc });


	// Textures - 2 per layer, but instead packed into two arrays of _numLayers for faster binding
	D3D11_TEXTURE2D_DESC hmDesc = Texture::create2DTexDesc(_texSize[0], _texSize[1], DXGI_FORMAT_R32_FLOAT,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_heightMap.create(device, &hmDesc, nullptr);

	D3D11_TEXTURE2D_DESC nmDesc = Texture::create2DTexDesc(2.f * _texSize[0], 2.f * _texSize[1], DXGI_FORMAT_R8G8B8A8_SNORM,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_normalMap.create(device, &nmDesc, nullptr);


	_coreSize = SVec2(_edgeVertCount * _coreVertSpacing);
	_coreOffset = SVec2(-0.5 * _coreSize);

	// Ring layer setup - towards bottom left per layer, starting with central block's offset
	SVec2 accumulatedSize(_coreSize);
	
	// Vertex spacing doubles for every ring layer, including the first
	float baseBlockSize = 2. * _blockEdgeVertCount * _coreVertSpacing;

	for (int i = 0; i < _layers.size(); ++i)
	{
		int scaleModifier = 1 << i;	// 1, 2, 4, 8...

		float blockSize = scaleModifier * baseBlockSize;
		
		RingLayer& rl = _layers[i];
		rl._blockSize = SVec2(blockSize);
		
		accumulatedSize += 2.f * rl._blockSize;	// Increase by the width of new blocks, one each side

		rl._size = accumulatedSize;
		rl._offset = -0.5 * accumulatedSize;
		
		// Each corner has 3 blocks, one in the corner, and two bordering it
		for (int j = 0; j < 4; j++)
		{
			// 2 3
			// 0 1
			SVec2 cornerOffset = rl._offset + SVec2(j & 1, j > 1) * (accumulatedSize - SVec2(blockSize));
			
			float xSign =  1. - 2. * (j % 2);	// 1 - 2 * (0, 1, 0, 1) = 1, -1, 1, -1
			float zSign = -1. + 2. * (j < 2);	// 1, 1, -1, -1
			rl._blockOffsets[j * 3 + 0] = cornerOffset;
			rl._blockOffsets[j * 3 + 1] = cornerOffset + SVec2(blockSize * xSign, 0);
			rl._blockOffsets[j * 3 + 2] = cornerOffset + SVec2(0, blockSize * zSign);
		}
	}
}



// @TODO replace with bufferless, no need for it on newer GPUs
void GeoClipmap::createVertexBuffers(ID3D11Device* device)
{
	// 12-block buffer
	std::vector<SVec2> vertXYs;
	vertXYs.reserve(_blockEdgeVertCount * _blockEdgeVertCount);

	std::vector<float> indices;

	for (int i = 0; i < _blockEdgeVertCount; ++i)
	{
		for (int j = 0; j < _blockEdgeVertCount; ++j)
			vertXYs.emplace_back(j, i);
	}

	VBuffer blockBuffer(device, vertXYs.data(), vertXYs.size() * sizeof(SVec2), sizeof(SVec2));
}



void GeoClipmap::update(ID3D11DeviceContext* context)
{

}



void GeoClipmap::draw(ID3D11DeviceContext* context)
{
}