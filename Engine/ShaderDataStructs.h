#pragma once
#include "Math.h"
#include <vector>
#include "Texture.h"
#include "lightclass.h"


struct SIBase 
{
public:
	SIBase() {};
	~SIBase() {};
};


struct MatrixBuffer : public SIBase
{
	SMatrix world;
	SMatrix view;
	SMatrix projection;
};


struct VariableBuffer : public SIBase
{
	float deltaTime;
	SVec3 padding;	//what a fucking waste of bandwidth gg microsoft
};


struct LightBuffer : public SIBase
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


struct DrawParams : public SIBase 
{
	SMatrix m, v, p;
	
	float deltaTime;

	PointLight pLight;
	SVec3 camPos;
};