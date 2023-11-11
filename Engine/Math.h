#pragma once

#include <d3d11_4.h>
#pragma warning(push)
#pragma warning(disable : 5260)
#include "SimpleMath.h"
#pragma warning(pop) 


using SMatrix	= DirectX::SimpleMath::Matrix;
using SQuat		= DirectX::SimpleMath::Quaternion;
using SVec2		= DirectX::SimpleMath::Vector2;
using SVec3		= DirectX::SimpleMath::Vector3;
using SVec4		= DirectX::SimpleMath::Vector4;
using SRay		= DirectX::SimpleMath::Ray;
using SPlane	= DirectX::SimpleMath::Plane;

static constexpr float PI = 3.1415926f;


class Math
{
public:

	static void RotateVecByQuat(SVec3& vec, const SQuat& rotator);

	static void RotateVecByMat(SVec3& vec, const SMatrix& rotatrix);

	static void RotateMatByQuat(SMatrix& transform, const SQuat& rotator);

	static void RotateMatByMat(SMatrix& transform, const SMatrix& rotatrix);

	static void RotationFromForwardUp(SMatrix& transform, const SVec3 fw, const SVec3 up);	 //might be incorrect

	static void SetRotation(SMatrix& transform, const SMatrix& rotatrix);

	static void Translate(SMatrix& transform, const SVec3& displacement);

	static void SetTranslation(SMatrix& transform, const SVec3& displacement);

	static void Scale(SMatrix& transform, const SVec3& xyz_factors);

	static void SetScale(SMatrix& transform, const SVec3& xyz_factors);

	static void CreatePerspectiveMatrix(SMatrix& projectionMatrix, float fov, float ar, float n, float f);

	static void CreatOrthoMatrix(SMatrix& orthographicMatrix, float w, float h, float n, float f);

	static void CamToViewMatrix(SMatrix& viewMatrix, const SMatrix& cameraMatrix);
	
	static SMatrix CalcMatrixStack(SMatrix& total, const SMatrix& model, const SMatrix& view, const SMatrix& projection);

	static SVec3 getNormalizedVec3(const SVec3& vecToNormalize);

	static SVec3 projectVecOntoVec(const SVec3& toProject, const SVec3& onto);

	static float clamp(float minX, float maxX, float x);

	static float smoothstep(float edge0, float edge1, float x);

	static void swap(float& f1, float& f2);

	static float remap(float value, float min1, float max1, float min2, float max2);

	static void rotateTowardsNoRoll(SMatrix& transform, const SVec3& target, float dTime);

	static SVec3 floor3(const SVec3& in);

	static float fract(const float in);

	static SVec3 fract3(const SVec3& in);

	inline static void setHeight(SMatrix& mat, float h)
	{
		mat._42 = h;
	}

	inline static SVec4 fromVec3(SVec3 v, float w)
	{
		return SVec4(v.x, v.y, v.z, w);
	}

	// This could all be wrong, I'm a bit rusty on space transitions since I haven't touched them in a while
	inline static SVec4 clipSpaceToViewSpace(const SVec4& clipSpace, const SMatrix& invProjMatrix)
	{
		return SVec4::Transform(clipSpace, invProjMatrix);
	}

	// Taken from a tutorial, seems fishy to me that w divide seems wrong!
	static SVec4 screenToViewSpace(const SVec4& ssCoords, const SVec2& sSize, const SMatrix& invProj)
	{
		SVec2 NDCoords = SVec2(ssCoords) / sSize;
		SVec4 clipSpace = SVec4(NDCoords.x * 2.f - 1.f, NDCoords.y * 2.f - 1.f, ssCoords.z, ssCoords.w);
		SVec4 viewSpace = clipSpaceToViewSpace(clipSpace, invProj);
		return (viewSpace / viewSpace.w);
	}

	static SVec2 resizeRetainAspectRatio(SVec2 itemDims, SVec2 containerDims)
	{
		float ratioW = containerDims.x / itemDims.x;
		float ratioH = containerDims.y / itemDims.y;
		float ratio = std::min(ratioW, ratioH);

		return ratio * itemDims;
	}

	// @WARNING just use taylor not sure if I got this right...
	inline static float sinBhaskara(float x)
	{
		return (16.f * x * (PI - x)) / (5.f * PI * PI - 4.f * x * (PI - x));
	}

	template <class T>
	static T lerp(const T a, const T b, const float t)
	{
		return (1 - t) * a + t * b;
	}
};



namespace cereal
{
	template<class Ar> void serialize(Ar& ar, SVec2& vec2) { ar(vec2.x, vec2.y); }

	template<class Ar> void serialize(Ar& ar, SVec3& vec3) { ar(vec3.x, vec3.y, vec3.z); }

	template<class Ar> void serialize(Ar& ar, SVec4& vec4) { ar(vec4.x, vec4.y, vec4.z, vec4.w); }

	template<class Ar> void serialize(Ar& ar, SQuat& quat) { ar(quat.x, quat.y, quat.z, quat.w); }

	template<class Ar> void serialize(Ar& ar, SMatrix& m)  { ar(m.m); }
};
