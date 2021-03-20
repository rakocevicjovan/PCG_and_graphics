#pragma once
#include "Math.h"
#include "Cone.h"
#include "CBuffer.h"
#include <cmath>

//#define INTENSITY_TO_RANGE(i) (sqrt(i / MIN_LIGHT))

// i / (r * r) = MIN_LIGHT; // multiply both sides by (r*r)
// i = MIN_LIGHT * (r * r);
// r * r = i /MIN_LIGHT;
// r =  sqrt(i / MIN_LIGHT);



class SceneLight
{
protected:
	SVec4 _chromaIntensity;

	//const float MIN_LIGHT = 0.05f;

	float INTENSITY_TO_RANGE(float i)
	{
		return  (sqrt(i / 0.03f));
	}

public:
	SceneLight() {}
	SceneLight(const SVec3& rgb, float intensity) : _chromaIntensity(rgb.x, rgb.y, rgb.z, intensity) {}
	SceneLight(const SVec4& rgbi) : _chromaIntensity(rgbi) {}

	SVec3 getChroma() { return SVec3(_chromaIntensity); }
	void setChroma(const SVec3& chroma) { _chromaIntensity = Math::fromVec3(chroma, _chromaIntensity.w); }
};



class DLight : public SceneLight
{
public:
	SVec4 _dir;	//takes care of cbuffer byte alignment on it's own... safest solution

	DLight() {}
	DLight(const SVec3& rgb, float intensity, const SVec3& dir) : SceneLight(rgb, intensity), _dir(Math::fromVec3(dir, 0)) {}
	DLight(const SVec4& rgbi, const SVec3& dir) : SceneLight(rgbi), _dir(Math::fromVec3(dir, 0)) {}
};



class PLight : public SceneLight
{
public:
	SVec4 _posRange;

	PLight() { sizeof(PLight); }

	PLight(const SVec3& rgb, float intensity, const SVec3& pos) 
		: SceneLight(rgb, intensity), _posRange(Math::fromVec3(pos, INTENSITY_TO_RANGE(intensity))) {}

	PLight(const SVec4& rgbi, const SVec3& pos) 
		: SceneLight(rgbi), _posRange(Math::fromVec3(pos, INTENSITY_TO_RANGE(rgbi.w))) {}
};



class SLight : public SceneLight
{
public:
	SVec4 _posRange;
	SVec4 _dirCosTheta;
	float _radius;

	SLight() { sizeof(SLight); }

	SLight(const SVec3& rgb, float intensity, const SVec3& pos, const SVec3& dir, float thetaRadians)
		: SceneLight(rgb, intensity), 
		_posRange(Math::fromVec3(pos, INTENSITY_TO_RANGE(intensity))), 
		_dirCosTheta(Math::fromVec3(dir, std::cosf(thetaRadians))),
		_radius(tan(thetaRadians) * _posRange.w)
	{}
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

	CBuffer _buffer;

	DirectionalLight(){}
	
	DirectionalLight(LightData ld, SVec4 dir) : LightData(ld), dir(dir) {}

	void createCBuffer(ID3D11Device* device)
	{
		_buffer.init(device, CBuffer::createDesc(sizeof(DirectionalLight) - sizeof(_buffer)));
	}

	void updateCBuffer(ID3D11DeviceContext* context)
	{
		_buffer.update(context, this, sizeof(DirectionalLight) - sizeof(_buffer));
	}

	void bind(ID3D11DeviceContext* context, uint8_t slot = 0ul)
	{
		_buffer.bindToPS(context, slot);
	}
};



struct PointLight : LightData
{
	SVec4 pos;

	CBuffer _buffer;

	PointLight(){}

	PointLight(LightData ld, SVec4 pos) : LightData(ld), pos(pos) {}

	// See how this feels to use then apply it to other lights if it's all right
	void createCBuffer(ID3D11Device* device)
	{
		_buffer.init(device, CBuffer::createDesc(sizeof(PointLight) - sizeof(_buffer)));
	}

	void updateCBuffer(ID3D11DeviceContext* context)
	{
		_buffer.update(context, this, sizeof(PointLight) - sizeof(_buffer));
	}

	void bind(ID3D11DeviceContext* context, uint8_t slot = 0ul)
	{
		_buffer.bindToPS(context, slot);
	}
};



struct SpotLight : PointLight
{
	SVec4 coneAxisAngle;	//minimal dot product being the w component of the vector

	SpotLight() {}
	SpotLight(PointLight pl, SVec3 coneAxis, float dotProdMin) : PointLight(pl), coneAxisAngle(coneAxisAngle) {}
};