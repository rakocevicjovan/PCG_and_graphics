#pragma once
#include "Math.h"
#include <vector>
#include <d3d11.h>
#include "Texture.h"
#include "Light.h"
#include "MeshDataStructs.h"


struct RenderFormat
{
	unsigned int stride = sizeof(Vert3D);
	unsigned int offset = 0u;
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};


struct MatrixBuffer
{
	SMatrix world;
	SMatrix view;
	SMatrix projection;
};


struct ShadowMatrixBuffer : public MatrixBuffer
{
	SMatrix lightView, lightProjection;
};


struct VariableBuffer
{
	float deltaTime;
	SVec3 padding;
};


struct LightBuffer
{
	SVec3 alc;
	float ali;
	SVec3  dlc;
	float dli;
	SVec3 slc;
	float sli;
	SVec4 pos;
	SVec4 ePos;
};

struct LightBuffer2
{
	SVec3 alc;
	float ali;
	SVec3  dlc;
	float dli;
	SVec3 slc;
	float sli;
	SVec4 pos;
};


struct ViewRayBuffer
{
	SMatrix rot;
	SVec4 ePos;
};


struct LightBufferType2
{
	SVec3 lightPosition;
	float padding;
};













//Shader parameters for frame by frame drawing

struct DrawParams
{
	SMatrix m, v, p;
	
	float deltaTime;

	PointLight pLight;
	SVec3 camPos;
};


class Model;
struct PointLight;

struct SPLight
{
	SPLight() {}

	ID3D11DeviceContext* deviceContext;
	SMatrix* modelMatrix;
	SMatrix* view;
	SMatrix* proj;
	PointLight* dLight;
	SVec3* eyePos;
	float deltaTime;
};