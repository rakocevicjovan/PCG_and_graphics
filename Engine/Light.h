#pragma once
#include "Math.h"
#include "Cone.h"
#include <cmath>

#define MIN_LIGHT 0.01
#define INTENSITY_TO_RANGE(i) (sqrt(i / MIN_LIGHT))

// i / (r * r) = MIN_LIGHT; // multiply both sides by (r*r)
// i = MIN_LIGHT * (r * r);
// r * r = i /MIN_LIGHT;
// r =  sqrt(i / MIN_LIGHT);



struct SceneLight
{
	SVec4 _chromaIntensity;

	SceneLight() {}
	SceneLight(const SVec3& rgb, float intensity) : _chromaIntensity(rgb.x, rgb.y, rgb.z, intensity) {}
	SceneLight(const SVec4& rgbi) : _chromaIntensity(rgbi) {}
};



struct DLight : public SceneLight
{
	SVec4 _dir;	//takes care of cbuffer byte alignment on it's own... safest solution

	DLight() {}
	DLight(const SVec3& rgb, float intensity, const SVec3& dir) : SceneLight(rgb, intensity), _dir(Math::fromVec3(dir, 0)) {}
	DLight(const SVec4& rgbi, const SVec3& dir) : SceneLight(rgbi), _dir(Math::fromVec3(dir, 0)) {}
};



struct PLight : public SceneLight
{
	SVec4 _posRange;

	PLight() { sizeof(PLight); }
	PLight(const SVec3& rgb, float intensity, const SVec3& pos) 
		: SceneLight(rgb, intensity), _posRange(Math::fromVec3(pos, INTENSITY_TO_RANGE(intensity))) {}

	PLight(const SVec4& rgbi, const SVec3& pos) 
		: SceneLight(rgbi), _posRange(Math::fromVec3(pos, INTENSITY_TO_RANGE(rgbi.w))) {}
};



struct SLight : public SceneLight
{
	SVec4 _posRange;
	SVec4 _dirCosTheta;

	SLight() { sizeof(Cone); }

	SLight(const SVec3& rgb, float intensity, const SVec3& pos, const SVec3& dir, float thetaRadians)
		: SceneLight(rgb, intensity), _posRange(Math::fromVec3(pos, INTENSITY_TO_RANGE(intensity))), _dirCosTheta(Math::fromVec3(dir, std::cosf(thetaRadians))) {}
};






struct LightData
{
	SVec3 alc;
	float ali;
	SVec3 dlc;
	float dli;
	SVec3 slc;
	float sli;

	LightData() {}


	LightData(SVec3 ambCol, float ambInt, SVec3 difCol, float difInt, SVec3 spcCol, float spcInt) 
		: alc(ambCol), ali(ambInt), dlc(difCol), dli(difInt), slc(spcCol), sli(spcInt) {}


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



/* // Circumvents same signatures but I don't really like it
	struct Angle { float theta; };
	SLight(const SVec3& rgb, float intensity, const SVec3& pos, const SVec3& dir, float cosTheta)
		: PLight(rgb, intensity, pos), _dirCosTheta(Math::fromVec3(dir, cosTheta)) {}

	SLight(const SVec3& rgb, float intensity, const SVec3& pos, const SVec3& dir, Angle thetaAngle)
		: PLight(rgb, intensity, pos), _dirCosTheta(Math::fromVec3(dir, std::cosf(thetaAngle.theta))) {}
*/