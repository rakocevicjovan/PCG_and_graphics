#pragma once
#include "Math.h"

struct LightData
{
	SVec3 alc;
	float ali;
	SVec3 dlc;
	float dli;
	SVec3 slc;
	float sli;

	LightData()
	{
	
	}


	LightData(SVec3 ambCol, float ambInt, SVec3 difCol, float difInt, SVec3 spcCol, float spcInt) 
		: alc(ambCol), ali(ambInt), dlc(difCol), dli(difInt), slc(spcCol), sli(spcInt)
	{

	}


	void Mix(const LightData other, float k, LightData& result) const
	{	
		result.alc = Math::lerp(alc, other.alc, k);
		result.ali = Math::lerp(ali, other.ali, k);
		result.dlc = Math::lerp(dlc, other.dlc, k);
		result.dli = Math::lerp(dli, other.dli, k);
		result.slc = Math::lerp(slc, other.slc, k);
		result.sli = Math::lerp(sli, other.sli, k);
	}
};



struct DirectionalLight : LightData
{
	SVec4 dir;
	
	DirectionalLight(){}
	
	DirectionalLight(LightData ld, SVec4 dir) : LightData(ld), dir(dir){}
};



struct PointLight : LightData
{
	SVec4 pos;

	PointLight(){}

	PointLight(LightData ld, SVec4 pos) : LightData(ld), pos(pos) {}
};



struct SpotLight : PointLight
{
	SVec4 coneAxisAngle;	//minimal dot product being the w component of the vector

	SpotLight() {}
	SpotLight(PointLight pl, SVec3 coneAxis, float dotProdMin) : PointLight(pl), coneAxisAngle(coneAxisAngle) {}
};