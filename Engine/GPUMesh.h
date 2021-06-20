#pragma once
#include "VBuffer.h"
#include "IBuffer.h"
#include "Mesh.h"

struct GPUMesh
{
	VBuffer _vertexBuffer{};
	IBuffer _indexBuffer{};

	static inline GPUMesh CreateFromMesh(ID3D11Device* device, Mesh& mesh)
	{
		GPUMesh gpuMesh;
		gpuMesh._vertexBuffer = VBuffer(device, mesh._vertices, mesh._vertSig, 0u);
		gpuMesh._indexBuffer = IBuffer(device, mesh._indices);
	}
};