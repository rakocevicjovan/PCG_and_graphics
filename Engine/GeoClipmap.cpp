#include "pch.h"
#include "GeoClipmap.h"
#include "VertSignature.h"
#include "TextureManager.h"


constexpr uint32_t textureBlockSize = 1 << 8;

// Calculate number of visible tiles in one direction rounded up 
int32_t determineVisibleDistanceInTiles(float farPlaneDistance)
{
	return static_cast<int32_t>(farPlaneDistance / textureBlockSize + 1);
}

std::pair<CellIndex, CellIndex> determineVisibleTileRange(SVec3 cameraPosition, int32_t farPlaneDistanceInTiles)
{
	// +8 is a hack for now, place in the middle of the map basically
	CellIndex cameraCell =
	{
		static_cast<int32_t>(cameraPosition.x / textureBlockSize) + 8,
		static_cast<int32_t>(cameraPosition.z / textureBlockSize) + 8
	};

	return
	{
		{
			cameraCell.x - farPlaneDistanceInTiles,
			cameraCell.y - farPlaneDistanceInTiles
		},
		{
			cameraCell.x + farPlaneDistanceInTiles,
			cameraCell.y + farPlaneDistanceInTiles
		}
	};
}

GeoClipmap::GeoClipmap(UINT numLayers, UINT edgeSizeLog2, float xzScale)
	: _numLayers(numLayers), _edgeVertCount(static_cast<uint32_t>(pow(2, edgeSizeLog2) - 1)), _coreVertSpacing(xzScale)
{
	_blockEdgeVertCount = (_edgeVertCount + 1) / 4;	// Size of the outer layer block in vertices
	_layers.resize(_numLayers);

	// Texture size
	_texSize = _edgeVertCount + 1;
}


void GeoClipmap::init(ID3D11Device* device)
{
	// DO THIS INSTEAD OF TEST DESC WHEN EVERYTHING IS FIXED
	// Vertex shader, constant buffer, input layout description
	VertSignature vertSig;
	vertSig.addAttribute(VAttribSemantic::POS, VAttribType::FLOAT2);	// Will be packed into float16
	std::vector<D3D11_INPUT_ELEMENT_DESC> inLayDesc = vertSig.createVertInLayElements();
	_vertShader = VertexShader(device, L"Shaders\\GeoClipmapVS.hlsl", inLayDesc, {});

	D3D11_BUFFER_DESC bufferDesc = CBuffer::createDesc(sizeof(GeoClipmapBuffer));
	_cBuffer.init(device, bufferDesc);

	createBuffers(device);
	createTextures(device);
	createTransformData();
}


// @TODO replace with bufferless, no need for it on newer GPUs (probably not worth it for the rim though)
void GeoClipmap::createBuffers(ID3D11Device* device)
{
	// Consider morton order? I'm not sure if it matters for vertices...

	// Reuse to save on allocation time, first one is the biggest anyways
	std::vector<SVec2> vertexData;
	
	// Central patch buffers
	createGridVertices(_edgeVertCount, _edgeVertCount, vertexData);
	_coreVB = VBuffer(device, vertexData.data(), vertexData.size() * sizeof(SVec2), sizeof(SVec2));
	_coreIB = IBuffer(device, createGridIndices(_edgeVertCount, _edgeVertCount));
	vertexData.clear();

	// Block buffer
	createGridVertices(_blockEdgeVertCount, _blockEdgeVertCount, vertexData);
	_blockVB = VBuffer(device, vertexData.data(), vertexData.size() * sizeof(SVec2), sizeof(SVec2));
	_blockIB = IBuffer(device, createGridIndices(_blockEdgeVertCount, _blockEdgeVertCount));
	vertexData.clear();


	// still puzzled, want to fit them into a single buffer but not make them too small 
	// or fpp could createa artefacts given the amount of scaling present in geo clipmaps,
	// but high scaling is always far away so might be invisible anyways...

	// Cross buffers
	std::vector<UINT> crossIndices;
	std::vector<UINT> extremityIndices;
	UINT extIdxCount = (_blockEdgeVertCount - 1) * (3 - 1) * 2 * 3;
	UINT extVertCount = _blockEdgeVertCount * 3;
	extremityIndices.reserve(extIdxCount);
	crossIndices.reserve(extIdxCount * 4);

	vertexData.reserve(extVertCount * 4);
	
	float squareWidth = _coreVertSpacing * 2.f;
	float blockWidth = squareWidth * (_blockEdgeVertCount - 1);
	float crossOffset = 3 * blockWidth + 2 * squareWidth;

	// Scale the entire structure to [0, 1] range but keep ratios
	float invTotalSize = 1 / (4 * blockWidth + 2 * squareWidth);
	squareWidth *= invTotalSize;
	blockWidth *= invTotalSize;
	crossOffset *= invTotalSize;

	// Left...
	createGridVertices(_blockEdgeVertCount, 3, vertexData);
	for (UINT i = 0; i < vertexData.size(); ++i)
	{
		vertexData[i] *= squareWidth;
		vertexData[i] -= SVec2(2 * blockWidth + 2 * squareWidth, 2 * squareWidth);
	}

	extremityIndices = createGridIndices(_blockEdgeVertCount, 3);
	crossIndices.insert(crossIndices.end(), extremityIndices.begin(), extremityIndices.end());

	// ... and right
	std::copy_n(vertexData.begin(), extVertCount, std::back_inserter(vertexData));
	for (UINT i = extVertCount; i < vertexData.size(); ++i)
	{
		vertexData[i].x += crossOffset;
	}

	for (UINT& idx : extremityIndices)
	{
		idx += extVertCount;
	}
	crossIndices.insert(crossIndices.end(), extremityIndices.begin(), extremityIndices.end());

	// Bottom...
	createGridVertices(3, _blockEdgeVertCount, vertexData);
	for (UINT i = extVertCount * 2; i < vertexData.size(); ++i)
	{
		vertexData[i] *= squareWidth;
		vertexData[i] -= SVec2(2 * squareWidth, 2 * blockWidth + 2 * squareWidth);
	}

	extremityIndices = createGridIndices(3, _blockEdgeVertCount);
	for (UINT& idx : extremityIndices)
	{
		idx += 2 * extVertCount;
	}
	crossIndices.insert(crossIndices.end(), extremityIndices.begin(), extremityIndices.end());

	// ... and top!
	std::copy_n(vertexData.begin() + 2 * extVertCount, extVertCount, std::back_inserter(vertexData));
	for (UINT i = 3 * extVertCount; i < vertexData.size(); ++i)
	{
		vertexData[i].y += crossOffset;
	}

	for (UINT& idx : extremityIndices)
	{
		idx += extVertCount;
	}
	crossIndices.insert(crossIndices.end(), extremityIndices.begin(), extremityIndices.end());

	_crossVB = VBuffer(device, vertexData.data(), vertexData.size() * sizeof(SVec2), sizeof(SVec2));
	_crossIB = IBuffer(device, crossIndices);
	vertexData.clear();
	
	// L rim buffer, built in bottom left initially, rotate around axis with +-(1, 1) scaling
	UINT rimVertLength = 2 * _blockEdgeVertCount + 1;

	// Vertical strip, erase last square
	createGridVertices(2, rimVertLength, vertexData);
	auto vStripIndices = createGridIndices(2, rimVertLength);
	// Erase last square from both, 4 vertices and 6 indices worth
	vertexData.resize(vertexData.size() - 4);
	vStripIndices.resize(vStripIndices.size() - 6);

	// Horizontal strip, offset indices as the vertices are all merged to a single array
	auto hStripIndices = createGridIndices(rimVertLength, 2);
	for (auto& idx : hStripIndices)
	{
		idx += vertexData.size();
	}
	createGridVertices(rimVertLength, 2, vertexData);

	// Merge and create
	std::vector<UINT> lRimIndices = std::move(vStripIndices);
	lRimIndices.reserve((8 + 7) * 2 * 3);	// 15 squares, 2 triangle faces each, 3 indices each
	std::copy(hStripIndices.begin(), hStripIndices.end(), std::back_inserter(lRimIndices));

	_rimVB = VBuffer(device, vertexData.data(), vertexData.size() * sizeof(SVec2), sizeof(SVec2));
	_rimIB = IBuffer(device, lRimIndices);
	vertexData.clear();

	// Degenerate triangles surrounding the layer (or inside it, who do I assign this to)
}


void GeoClipmap::createTextures(ID3D11Device* device)
{
	// Textures - 2 per layer, but instead packed into two arrays of _numLayers for faster binding
	D3D11_TEXTURE2D_DESC hmDesc = Texture::Create2DTexDesc(_texSize, _texSize, DXGI_FORMAT_R32_FLOAT,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u, _numLayers);
	
	_heightMap.create(device, hmDesc, nullptr, true);

	D3D11_TEXTURE2D_DESC nmDesc = Texture::Create2DTexDesc(2 * _texSize, 2 * _texSize, DXGI_FORMAT_R8G8B8A8_SNORM,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_normalMap.create(device, nmDesc, nullptr, true);
}


void GeoClipmap::createTransformData()
{
	// Core size is increased by 2*_coreVertSpacing because of the L-strip (which is wider)
	_coreSize = SVec2((_edgeVertCount + 1) * _coreVertSpacing);
	_coreOffset = SVec2(-0.5 * _coreSize);

	float baseVertSpacing = 2.f * _coreVertSpacing;	// Vertex spacing doubles for every ring layer
	float baseBlockSize = (_blockEdgeVertCount - 1) * baseVertSpacing;
	float baseCrossWidth = 2.f * baseVertSpacing;	// Three vertices, two gaps of spacing.

	for (int i = 0; i < _layers.size(); ++i)
	{
		int scaleModifier = 1 << i;

		float vertSpacing = baseVertSpacing * scaleModifier;
		float blockSize = baseBlockSize * scaleModifier;
		float crossWidth = baseCrossWidth * scaleModifier;

		RingLayer& rl = _layers[i];

		rl._blockSize = SVec2(blockSize);
		rl._vertexScale = SVec2(vertSpacing);

		rl._size = SVec2(4 * blockSize + crossWidth);

		// Each layer is initially shifted bottom left, to leave space for the L-strip
		// Meaning each layer is in the top right corner of the bigger layer
		rl._offset = SVec2(-2 * blockSize - 2 * vertSpacing);

		// Each corner has 3 blocks, one in the corner, and two bordering it
		for (int j = 0; j < 4; j++)
		{
			// 2 3
			// 0 1
			SVec2 cornerOffset = rl._offset + SVec2(static_cast<float>(j & 1), static_cast<float>(j > 1)) * (rl._size - rl._blockSize);

			float xSign = static_cast<float>(1. - 2. * (j % 2));	// 1 - 2 * (0, 1, 0, 1) = 1, -1, 1, -1
			float zSign = static_cast<float>(-1. + 2. * (j < 2));	// 1, 1, -1, -1

			rl._blockOffsets[j * 3 + 0] = cornerOffset;
			rl._blockOffsets[j * 3 + 1] = cornerOffset + SVec2(blockSize * xSign, 0);
			rl._blockOffsets[j * 3 + 2] = cornerOffset + SVec2(0, blockSize * zSign);
		}
	}
}


void GeoClipmap::createGridVertices(UINT numCols, UINT numRows, std::vector<SVec2>& output)
{
	UINT requiredSize = numCols * numRows;

	output.reserve(requiredSize);

	for (UINT z = numRows; z > 0; --z)
	{
		for (UINT x = 0; x < numCols; ++x)
		{
			output.emplace_back(static_cast<float>(x), static_cast<float>(z - 1));
		}
	}
}


std::vector<UINT> GeoClipmap::createGridIndices(UINT numCols, UINT numRows)
{
	UINT x = numCols - 1;
	UINT z = numRows - 1;

	std::vector<UINT> indices;
	indices.reserve(x * z * 2 * 3);	// Rows and columns of squares, two triangles each, 3 UINTs each

	for (UINT i = 0; i < z; ++i)		// For every row
	{
		for (UINT j = 0; j < x; ++j)	// For every column in the row
		{
			UINT tli = i * numCols + j;
			UINT tri = tli + 1;
			UINT bli = tli + numCols;
			UINT bri = bli + 1;

			indices.insert(indices.end(), { tli, tri, bli, bli, tri, bri });
		}
	}

	return indices;
}


void GeoClipmap::update(ID3D11DeviceContext* context, SVec3 cameraPosition, float farPlaneDistance)
{
	// Fetch tiles we need but don't have yet
	const auto farPlaneDistanceInTiles = determineVisibleDistanceInTiles(farPlaneDistance);

	// This might shrink often if the distance varies but not worried about it for now, distance shouldn't vary much
	const auto gridSize = 2 * farPlaneDistanceInTiles;
	_chunkTextures.reserve(gridSize * gridSize);

	// Not expected to move more than 1 row and 1 column at once
	//std::vector<CellIndex> tilesToRemove;
	//tilesToRemove.reserve(2 * gridSize - 1);
	std::vector<std::pair<CellIndex, std::shared_future<std::shared_ptr<Texture>>>> tilesToAdd;
	tilesToAdd.reserve(2 * gridSize - 1);

	const auto [min, max] = determineVisibleTileRange(cameraPosition, farPlaneDistanceInTiles);
	const auto& [prevMin, prevMax] = _prevVisibleRange;

	//const auto addRowTo =
	//	[&](uint32_t val, std::vector<CellIndex>& container)
	//	{
	//		for (auto i = 0u; i < gridSize; ++i)
	//		{
	//			container.emplace_back(i, val);
	//		}
	//	};
	//
	//const auto addColTo =
	//	[&](uint32_t val, std::vector<CellIndex>& container)
	//{
	//	for (auto i = 0u; i < gridSize; ++i)
	//	{
	//		container.emplace_back(val, i);
	//	}
	//};
	//
	//{
	//	uint32_t xBegin{ };
	//	uint32_t xEnd{  };
	//
	//	const bool min_moved_right = min.x > prevMin.x;
	//	//const int xDir = min_moved_right ? 1 : -1;
	//	if (min_moved_right)
	//	{
	//		xBegin = prevMin.x;
	//		xEnd = min.x;
	//
	//		// Left columns removed
	//		for (auto x = xBegin; x < xEnd; ++x)
	//		{
	//			addColTo(x, tilesToRemove);
	//		}
	//	}
	//	else
	//	{
	//		xBegin = min.x;
	//		xEnd = prevMin.x;
	//
	//		// Left columns added
	//		for (auto x = xBegin; x < xEnd; ++x)
	//		{
	//			addColTo(x, tilesToAdd);
	//		}
	//	}
	//
	//	const bool max_moved_right = max.x > prevMax.x;
	//	if (max_moved_right)
	//	{
	//		xBegin = prevMax.x;
	//		xEnd = max.x;
	//	}
	//	else
	//	{
	//		xBegin = max.x;
	//		xEnd = prevMax.x;
	//	}
	//
	// etc etc... lengthy
	//}

	// Deactivate unused textures, to be erased in time
	for (auto& [cell, texture] : _chunkTextures)
	{
		if (cell.x < min.x || cell.y < min.y || cell.x > max.x || cell.y > max.y)
		{
			constexpr auto frame_timeout = 1 << 16;
			if (++texture.sinceLastUse > frame_timeout)
			{
				_chunkTextures.erase(cell);
			}
		}
	}

	// Add new textures, marked as active
	for (auto x = min.x; x < max.x; ++x)
	{
		for (auto y = min.y; y < max.y; ++y)
		{
			const auto cellIndex = CellIndex{ x, y };
			const auto [kvPair, isNewInsert] = _chunkTextures.try_emplace(cellIndex, ActiveTexture{ 0, {} });
			if (isNewInsert)
			{
				tilesToAdd.push_back({ cellIndex, std::shared_future<std::shared_ptr<Texture>>{} });
			}
			else
			{
				kvPair->second.sinceLastUse = 0;
			}
		}
	}

	std::string name;
	name.resize(150);

	// Better than a series of blocking calls, we let all loads run in parallel. 
	for (auto& [cellIndex, future] : tilesToAdd)
	{
		std::sprintf(name.data(), "C:\\Users\\metal\\Desktop\\geoclipmap\\tiles\\mountain_%02d_%02d.png", cellIndex.x, cellIndex.y);
		future = _textureManager->getAsync(name.c_str());
	}

	// Still blocks until the batch finishes. This can be better but ok for now.
	for (auto& [cellIndex, future] : tilesToAdd)
	{
		future.wait();
		_chunkTextures[cellIndex].texture = future.get();
	}

	// To determine if 1 is required or not, snap camera

	// 1. Load (and decompress?) the new texture region, if required
	// 2. Run a pixel shader to update the the gpu texture
	// 3. Update the sampling offsets for toroidal access
}


void GeoClipmap::draw(ID3D11DeviceContext* context)
{
	// Bind once, no need to repeat as the same state is used everywhere
	_vertShader.bind(context);
	context->PSSetShader(nullptr, nullptr, 0);
	_cBuffer.bindToVS(context, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Core rendering
	context->IASetVertexBuffers(0, 1, _coreVB.ptr(), &_coreVB._stride, &_coreVB._offset);
	context->IASetIndexBuffer(_coreIB.ptr(), DXGI_FORMAT_R32_UINT, 0);

	_bufferData.scaleTranslation.x = _coreVertSpacing;
	_bufferData.scaleTranslation.y = _coreVertSpacing;
	_bufferData.scaleTranslation.z = _coreOffset.x;
	_bufferData.scaleTranslation.w = _coreOffset.y;
	_cBuffer.updateWithStruct(context, _bufferData);
	context->DrawIndexed(_coreIB.getIdxCount(), 0, 0);

	// Block rendering
	context->IASetVertexBuffers(0, 1, _blockVB.ptr(), &_blockVB._stride, &_blockVB._offset);
	context->IASetIndexBuffer(_blockIB.ptr(), DXGI_FORMAT_R32_UINT, 0);

	for (UINT i = 0; i < _numLayers; ++i)
	{
		RingLayer& rl = _layers[i];
		
		_bufferData.scaleTranslation.x = rl._vertexScale.x;
		_bufferData.scaleTranslation.y = rl._vertexScale.y;

		for (const SVec2& offset : rl._blockOffsets)
		{
			_bufferData.scaleTranslation.z = offset.x;
			_bufferData.scaleTranslation.w = offset.y;
			_cBuffer.updateWithStruct(context, _bufferData);
			context->DrawIndexed(_blockIB.getIdxCount(), 0, 0);
		}
	}


	// Cross rendering
	context->IASetVertexBuffers(0, 1, _crossVB.ptr(), &_crossVB._stride, &_crossVB._offset);
	context->IASetIndexBuffer(_crossIB.ptr(), DXGI_FORMAT_R32_UINT, 0);

	for (UINT i = 0; i < _numLayers; ++i)
	{
		_bufferData.scaleTranslation = SVec4(_layers[i]._size.x, _layers[i]._size.y, 0., 0.);
		_cBuffer.updateWithStruct(context, _bufferData);
		context->DrawIndexed(_crossIB.getIdxCount(), 0, 0);
	}


	// L-trim rendering
	context->IASetVertexBuffers(0, 1, _rimVB.ptr(), &_rimVB._stride, &_rimVB._offset);
	context->IASetIndexBuffer(_rimIB.ptr(), DXGI_FORMAT_R32_UINT, 0);

	for (UINT i = 0; i < _numLayers; ++i)
	{
		_bufferData.scaleTranslation.x = _layers[i]._vertexScale.x;
		_bufferData.scaleTranslation.y = _layers[i]._vertexScale.x;
		
		_bufferData.scaleTranslation.z = _layers[i]._offset.x + _layers[i]._blockSize.x;
		_bufferData.scaleTranslation.w = _layers[i]._offset.y + _layers[i]._blockSize.y;

		_cBuffer.updateWithStruct(context, _bufferData);
		context->DrawIndexed(_rimIB.getIdxCount(), 0, 0);
	}

	/*
	D3D11_BUFFER_DESC instanceBufferDesc =
		shc.createBufferDesc(sizeof(InstanceData) * instanceBufferSizeInElements,
			D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE);
	
	// Use instancing later, try to pack the above into a single cbuffer for all instances, should fit easily
	//context->DrawIndexedInstanced(_coreIB.getIdxCount(), _numLayers * 12, 0, 0, 0);
	*/
}


// @TODO
SVec2 GeoClipmap::getLevelOffset(const SVec2& camPos, UINT level)
{
	//int scale = 1 << level;
	//SVec2 scaledPos = camPos / SVec2(scale); // Snap to grid in the appropriate space.

	// Snap to grid of next level. I.e. we move the clipmap level in steps of two.
	//SVec2 snapped_pos = SVec2(scaledPos / SVec2(1 << (level + 1))) * SVec2(1 << (level + 1));
	
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