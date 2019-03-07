#pragma once
#include <d3d11.h>
#include "SimpleMath.h"

using SMatrix = DirectX::SimpleMath::Matrix;
using SQuat = DirectX::SimpleMath::Quaternion;
using SVec2 = DirectX::SimpleMath::Vector2;
using SVec3 = DirectX::SimpleMath::Vector3;
using SVec4 = DirectX::SimpleMath::Vector4;
using SRay = DirectX::SimpleMath::Ray;

static const float PI = 3.1415926f;


class Math{

public:
	
	Math();

	~Math();

	static void RotateVecByQuat(SVec3& vec, const SQuat& rotator) {
		vec = SVec3::Transform(vec, rotator);
	}

	static void RotateVecByMat(SVec3& vec, const SMatrix& rotatrix) {
		vec = SVec3::Transform(vec, rotatrix);
	}

	static void RotateMatByQuat(SMatrix& transform, const SQuat& rotator) {
		transform = SMatrix::Transform(transform, rotator);
	}

	static void RotateMatByMat(SMatrix& transform, const SMatrix& rotatrix) {
		transform = transform * rotatrix;	//might need to inverse the order!!! this is different than openGL because of column vs row bs...
	}

	 //might be badde
	static void RotationFromForwardUp(SMatrix& transform, const SVec3 fw, const SVec3 up) {

		SVec3 right = fw.Cross(up);
		right.Normalize();

		transform = DirectX::XMMatrixSet( 
			right.x, right.y, right.z, 0,
			up.x, up.y, up.z, 0,
			fw.x, fw.y, fw.z, 0,
			transform._41, transform._42, transform._43, 1);
	}

	static void SetRotation(SMatrix& transform, const SMatrix& rotatrix)
	{
		SVec3 pos = transform.Translation();
		transform = rotatrix;
		transform *= SMatrix::CreateTranslation(pos);	//might be wrong
	}

	static void Translate(SMatrix& transform, const SVec3& displacement)
	{
		SetTranslation(transform,  transform.Translation() + displacement);
	}

	static void SetTranslation(SMatrix& transform, const SVec3& displacement)
	{
		transform._41 = displacement.x;
		transform._42 = displacement.y;
		transform._43 = displacement.z;
	}

	//only use when not rotated/translated... should wrap all that into a function as well
	static void Scale(SMatrix& transform, const SVec3& xyz_factors)
	{
		transform = transform * SMatrix::CreateScale(xyz_factors);
	}

	static void CreatePerspectiveMatrix(SMatrix& projectionMatrix, float fov, float ar, float n, float f)
	{
		projectionMatrix = SMatrix::CreatePerspectiveFieldOfView(fov, ar, n, f);
	}

	static void CreatOrthoMatrix(SMatrix& orthographicMatrix, float w, float h, float n, float f)
	{
		orthographicMatrix = SMatrix::CreateOrthographic(w, h, n, f);
	}

	static void CamToViewMatrix(SMatrix& viewMatrix, const SMatrix& cameraMatrix)
	{
		viewMatrix = cameraMatrix.Invert();
	}
	
	static SMatrix CalcMatrixStack(SMatrix& total, const SMatrix& model, const SMatrix& view, const SMatrix& projection)
	{
		total =  model * view * projection;
	}

	static SVec3 getNormalizedVec3(const SVec3& vecToNormalize) 
	{
		return vecToNormalize / vecToNormalize.Length();
	}

	static SVec3 projectVecOntoVec(const SVec3& toProject, const SVec3& onto)
	{
		return ( onto.Dot(toProject) / onto.Dot(onto) ) * onto;
	}

	static SVec4 fromVec3(SVec3 v, float w)
	{
		return SVec4(v.x, v.y, v.z, w);
	}

};

