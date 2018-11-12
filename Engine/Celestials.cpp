#include "Celestials.h"



Celestials::Celestials(){

	LightData SunlightData;
	SunlightData.alc = SVec3(1.0f, 1.0f, 1.0f);
	SunlightData.ali = 0.2f;
	SunlightData.dlc = SunlightData.alc;
	SunlightData.dli = 0.5f;
	SunlightData.slc = SunlightData.alc;
	SunlightData.sli = 1.0f;

	SVec4 SunLightPos = SVec4(0.0f, 1.0f, 0.0f, 1.0f);

	Sun = PointLight(SunlightData, SunLightPos);


	LightData MoonlightData;
	MoonlightData.alc = SVec3(0.3f, 0.6f, 0.9f);
	MoonlightData.ali = 0.1f;
	MoonlightData.dlc = MoonlightData.alc;
	MoonlightData.dli = 0.5f;
	MoonlightData.slc = MoonlightData.alc;
	MoonlightData.sli = 1.0f;

	SVec4 MoonLightPos = SVec4(0.0f, -1.0f, 0.0f, 1.0f);

	Moon = PointLight(MoonlightData, MoonLightPos);


	Warm.alc = SVec3(1.0f, 0.66f, 0.66f);
	Warm.ali = 0.1f;
	Warm.dlc = MoonlightData.alc;
	Warm.dli = 0.5f;
	Warm.slc = MoonlightData.alc;
	Warm.sli = 1.0f;

	Current = Sun;
}



void Celestials::Interpolate(PointLight pl1, PointLight pl2, float TOD) {

	float nightCoefficient = TOD / 24.f;

	pl1.Mix(pl2, nightCoefficient, Current);
}