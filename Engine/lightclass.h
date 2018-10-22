#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_

#include "Math.h"

struct LightData {

public:

	SVec3 alc;
	float ali;
	SVec3  dlc;
	float dli;
	SVec3 slc;
	float sli;

	LightData() {}
	LightData(SVec3 ambCol, float ambInt, SVec3 difCol, float difInt, SVec3 spcCol, float spcInt) 
		: alc(ambCol), ali(ambInt), dlc(difCol), dli(difInt), slc(spcCol), sli(spcInt) {}
};



struct DirectionalLight : public LightData {
	SVec4 dir;
	
	DirectionalLight(){}
	
	DirectionalLight(LightData ld, SVec4 dir) : LightData(ld), dir(dir){}
};



struct PointLight : LightData {
	SVec4 pos;

	PointLight() {}

	PointLight(LightData ld, SVec3 pos) : LightData(ld), pos(pos) {}
};



struct SpotLight : PointLight {
	SVec3 coneAxis;
	float dotProdMin;

	SpotLight() {}
	SpotLight(PointLight pl, SVec3 coneAxis, float dotProdMin) : PointLight(pl), coneAxis(coneAxis), dotProdMin(dotProdMin) {}
};

#endif