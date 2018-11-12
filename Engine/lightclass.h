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

	void Mix(const LightData other, float k, LightData& result) const{
		
		result.alc = alc * (1 - k) + other.alc * k;
		result.ali = ali * (1 - k) + other.ali * k;
		result.dlc = dlc * (1 - k) + other.dlc * k;
		result.dli = dli * (1 - k) + other.dli * k;
		result.slc = slc * (1 - k) + other.slc * k;
		result.sli = sli * (1 - k) + other.sli * k;
	}
};



struct DirectionalLight : public LightData {
	SVec4 dir;
	
	DirectionalLight(){}
	
	DirectionalLight(LightData ld, SVec4 dir) : LightData(ld), dir(dir){}
};



struct PointLight : LightData {
	SVec4 pos;

	PointLight() {}

	PointLight(LightData ld, SVec4 pos) : LightData(ld), pos(pos) {}
};



struct SpotLight : PointLight {
	SVec3 coneAxis;
	float dotProdMin;

	SpotLight() {}
	SpotLight(PointLight pl, SVec3 coneAxis, float dotProdMin) : PointLight(pl), coneAxis(coneAxis), dotProdMin(dotProdMin) {}
};

#endif