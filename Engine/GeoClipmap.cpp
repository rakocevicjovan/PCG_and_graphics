#include "GeoClipmap.h"
#include "VertSignature.h"



GeoClipmap::GeoClipmap(UINT numLayers, UINT edgeSizeLog2, float xzScale)
	: _numLayers(numLayers), _edgeVertCount(pow(2, edgeSizeLog2) - 1), _coreVertSpacing(xzScale)
{
	_blockEdgeVertCount = (_edgeVertCount + 1) / 4;	// Size of the outer layer block in vertices
	_gapSize = _blockEdgeVertCount * 3;	// Cardinal gaps
	_layers.resize(_numLayers);

	// Texture size
	_texSize = _edgeVertCount;
}



void GeoClipmap::init(ID3D11Device* device)
{
	// DO THIS INSTEAD OF TEST DESC WHEN EVERYTHING IS FIXED
	// Vertex shader, constant buffer, input layout description
	//VertSignature vertSig;
	//vertSig.addAttribute(VAttribSemantic::POS, VAttribType::FLOAT2);	// Will be packed into float16
	//std::vector<D3D11_INPUT_ELEMENT_DESC> inLayDesc = vertSig.createVertInLayElements();

	D3D11_INPUT_ELEMENT_DESC testDesc { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	//_vertShader = VertexShader(device, L"GeoClipmapVS.hlsl", { testDesc }, {});
	ShaderCompiler shc(device);
	auto blob = shc.compileToBlob(L"GeoClipmapVS.hlsl", "vs_5_0");
	if (FAILED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &_vertShader._vsPtr)))
		__debugbreak();

	if (FAILED(device->CreateInputLayout(&testDesc, 1, blob->GetBufferPointer(), blob->GetBufferSize(), &_vertShader._layout)))
		__debugbreak();

	D3D11_BUFFER_DESC bufferDesc = CBuffer::createDesc(sizeof(GeoClipmapBuffer));
	_cBuffer.createBuffer(device, bufferDesc, _cBuffer._cbPtr);

	// Buffers for all the parts
	createBuffers(device);

	/*
	// Textures - 2 per layer, but instead packed into two arrays of _numLayers for faster binding
	D3D11_TEXTURE2D_DESC hmDesc = Texture::create2DTexDesc(_texSize, _texSize, DXGI_FORMAT_R32_FLOAT,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_heightMap.create(device, &hmDesc, nullptr);

	D3D11_TEXTURE2D_DESC nmDesc = Texture::create2DTexDesc(2.f * _texSize, 2.f * _texSize, DXGI_FORMAT_R8G8B8A8_SNORM,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_normalMap.create(device, &nmDesc, nullptr);
	*/

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



// @TODO replace with bufferless, no need for it on newer GPUs (probably not worth it for the rim though)
void GeoClipmap::createBuffers(ID3D11Device* device)
{
	// Consider morton order? I'm not sure if it matters for vertices...

	// Central patch buffers
	std::vector<SVec2> vertexData;	// Reuse everywhere
	for (UINT i = 0; i < _edgeVertCount; ++i)
	{
		for (UINT j = 0; j < _edgeVertCount; ++j)
			vertexData.emplace_back(j, _edgeVertCount - i - 1);
	}

	_coreVB = VBuffer(device, vertexData.data(), vertexData.size() * sizeof(SVec2), sizeof(SVec2));
	_coreIB = IBuffer(device, createGridIndices(_edgeVertCount, _edgeVertCount));

	vertexData.clear();

	// Block buffer
	for (UINT i = 0; i < _blockEdgeVertCount; ++i)
	{
		for (UINT j = 0; j < _blockEdgeVertCount; ++j)
			vertexData.emplace_back(j, _blockEdgeVertCount - i - 1);
	}

	_blockVB = VBuffer(device, vertexData.data(), vertexData.size() * sizeof(SVec2), sizeof(SVec2));
	_blockIB = IBuffer(device, createGridIndices(_blockEdgeVertCount, _blockEdgeVertCount));

	vertexData.clear();
	
	// L rim buffer, built in bottom left initially
	UINT lineStripSize = (2 * _blockEdgeVertCount + 1) * 2;
	UINT rimSize = 2 * lineStripSize - 4;	// Duplicates, L strip only needs 32 verts if merged together

	for (UINT i = 0; i < 18; ++i)
		vertexData.emplace_back(i & 1, i / 2);			// Vertical strip, left-right-up...

	for (UINT i = 18; i < rimSize; ++i)
		vertexData.emplace_back(2 + (i / 2), i & 1);	// Horizontal strip, bottom-up-right...

	std::vector<UINT> lRimIndices;
	UINT numRimIndices = (8 + 7) * 2 * 3;	// 15 squares, 2 triangle faces each, 3 indices each
	lRimIndices.reserve(numRimIndices);

	for (UINT i = 0; i < 15 * 4; i += 4)	// 4 vertices per square to cover
		lRimIndices.insert(lRimIndices.end(), {i + 2, i + 3, i, i, i + 3, i + 1});

	_rimVB = VBuffer(device, vertexData.data(), vertexData.size() * sizeof(SVec2), sizeof(SVec2));
	_rimIB = IBuffer(device, lRimIndices);

	// Degenerate triangles surrounding the layer (or inside it, who do I assign this to)
}



void GeoClipmap::update(ID3D11DeviceContext* context)
{

}



void GeoClipmap::draw(ID3D11DeviceContext* context)
{
	context->VSSetShader(_vertShader._vsPtr, NULL, 0);
	context->PSSetShader(NULL, NULL, 0);

	context->IASetInputLayout(_vertShader._layout);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetVertexBuffers(0, 1, _blockVB.ptr(), &_blockVB._stride, &_blockVB._offset);
	context->IASetIndexBuffer(_blockIB.ptr(), DXGI_FORMAT_R32_UINT, 0);

	
	float scale = 2.f;
	for (UINT i = 0; i < _numLayers; ++i)
	{
		RingLayer& rl = _layers[i];
		
		_bufferData.scaleTranslation.x = _bufferData.scaleTranslation.y = scale;
		scale *= 2.f;

		for (const SVec2& offset : rl._blockOffsets)
		{
			_bufferData.scaleTranslation.z = offset.x;
			_bufferData.scaleTranslation.w = offset.y;

			// Update and set cbuffers
			_cBuffer.updateWithStruct(context, _cBuffer._cbPtr, _bufferData);
			context->VSSetConstantBuffers(0, 1, &_cBuffer._cbPtr);

			context->DrawIndexed(_blockIB.getIdxCount(), 0, 0);
		}
	}
	

	// Use instancing later
	//context->DrawIndexedInstanced(_coreIB.getIdxCount(), _numLayers * 12, 0, 0, 0);
}



std::vector<UINT> GeoClipmap::createGridIndices(UINT numCols, UINT numRows)
{
	UINT x = numCols - 1;
	UINT y = numRows - 1;

	std::vector<UINT> indices;
	indices.reserve(x * y * 2 * 3);	// Rows and columns of squares, two triangles each, 3 UINTs each

	for (UINT i = 0; i < y; ++i)		// For every row
	{
		for (UINT j = 0; j < x; ++j)	// For every column in the row
		{
			UINT tli = i * numCols + j;
			UINT tri = tli + 1;
			UINT bli = tli + numCols;
			UINT bri = bli + 1;

			indices.insert(indices.end(), { tli, tri, bli });
			indices.insert(indices.end(), { bli, tri, bri });
		}
	}

	return indices;
}



// @TODO
SVec2 GeoClipmap::getLevelOffset(const SVec2& camera_pos, unsigned int level)
{
	int scale = 1 << level;
	//SVec2 scaled_pos = camera_pos / SVec2(clipmap_scale); // Snap to grid in the appropriate space.

	// Snap to grid of next level. I.e. we move the clipmap level in steps of two.
	//SVec2 snapped_pos = SVec2(scaled_pos / vec2(1 << (level + 1))) * vec2(1 << (level + 1));
	
	// Apply offset so all levels align up neatly.
	// If snapped_pos is equal for all levels,
	// this causes top-left vertex of level N to always align up perfectly with top-left interior corner of level N + 1.
	// This gives us a bottom-right trim region.
	// Due to the flooring, snapped_pos might not always be equal for all levels.
	// The flooring has the property that snapped_pos for level N + 1 is less-or-equal snapped_pos for level N.
	// If less, the final position of level N + 1 will be offset by -2 ^ N, which can be compensated for with changing trim-region to top-left.
	//SVec2 pos = snapped_pos - vec2((2 * (size - 1)) << level);
	
	SVec2 dummy;
	return dummy;
}