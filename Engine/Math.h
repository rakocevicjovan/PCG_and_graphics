#pragma once
#include <d3d11.h>
#include "SimpleMath.h"

using SMatrix = DirectX::SimpleMath::Matrix;
using SQuat		= DirectX::SimpleMath::Quaternion;
using SVec2		= DirectX::SimpleMath::Vector2;
using SVec3		= DirectX::SimpleMath::Vector3;
using SVec4		= DirectX::SimpleMath::Vector4;
using SRay		= DirectX::SimpleMath::Ray;
using SPlane	= DirectX::SimpleMath::Plane;

static const float PI = 3.1415926f;


class Math{

public:
	
	Math();

	~Math();

	static void RotateVecByQuat(SVec3& vec, const SQuat& rotator);

	static void RotateVecByMat(SVec3& vec, const SMatrix& rotatrix);

	static void RotateMatByQuat(SMatrix& transform, const SQuat& rotator);

	static void RotateMatByMat(SMatrix& transform, const SMatrix& rotatrix);

	static void RotationFromForwardUp(SMatrix& transform, const SVec3 fw, const SVec3 up);	 //might be incorrect

	static void SetRotation(SMatrix& transform, const SMatrix& rotatrix);

	static void Translate(SMatrix& transform, const SVec3& displacement);

	static void SetTranslation(SMatrix& transform, const SVec3& displacement);

	static void Scale(SMatrix& transform, const SVec3& xyz_factors);

	static void CreatePerspectiveMatrix(SMatrix& projectionMatrix, float fov, float ar, float n, float f);

	static void CreatOrthoMatrix(SMatrix& orthographicMatrix, float w, float h, float n, float f);

	static void CamToViewMatrix(SMatrix& viewMatrix, const SMatrix& cameraMatrix);
	
	static SMatrix CalcMatrixStack(SMatrix& total, const SMatrix& model, const SMatrix& view, const SMatrix& projection);

	static SVec3 getNormalizedVec3(const SVec3& vecToNormalize);

	static SVec3 projectVecOntoVec(const SVec3& toProject, const SVec3& onto);

	static SVec4 fromVec3(SVec3 v, float w);

	static float clamp(float minX, float maxX, float x);

	static float smoothstep(float edge0, float edge1, float x);

	static void swap(float& f1, float& f2);

	static float remap(float value, float min1, float max1, float min2, float max2);

};

