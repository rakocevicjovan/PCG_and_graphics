#include "Math.h"

Math::Math()
{
	
}


Math::~Math()
{
}



void Math::RotateVecByQuat(SVec3& vec, const SQuat& rotator)
{
	vec = SVec3::Transform(vec, rotator);
}

void Math::RotateVecByMat(SVec3& vec, const SMatrix& rotatrix)
{
	vec = SVec3::Transform(vec, rotatrix);
}

void Math::RotateMatByQuat(SMatrix& transform, const SQuat& rotator)
{
	transform = SMatrix::Transform(transform, rotator);
}

void Math::RotateMatByMat(SMatrix& transform, const SMatrix& rotatrix)
{
	transform = transform * rotatrix;
}

void Math::RotationFromForwardUp(SMatrix& transform, const SVec3 fw, const SVec3 up)
{
	SVec3 right = fw.Cross(up);
	right.Normalize();

	transform = DirectX::XMMatrixSet(
		right.x, right.y, right.z, 0,
		up.x, up.y, up.z, 0,
		fw.x, fw.y, fw.z, 0,
		transform._41, transform._42, transform._43, 1);
}

void Math::SetRotation(SMatrix& transform, const SMatrix& rotatrix)
{
	SVec3 pos = transform.Translation();
	transform = rotatrix;
	transform *= SMatrix::CreateTranslation(pos);
}

void Math::Translate(SMatrix& transform, const SVec3& displacement)
{
	SetTranslation(transform, transform.Translation() + displacement);
}

void Math::SetTranslation(SMatrix& transform, const SVec3& displacement)
{
	transform._41 = displacement.x;
	transform._42 = displacement.y;
	transform._43 = displacement.z;
}

//only use when not rotated/translated... should wrap all that into a function as well
void Math::Scale(SMatrix& transform, const SVec3& xyz_factors)
{
	transform = transform * SMatrix::CreateScale(xyz_factors);
}

void Math::CreatePerspectiveMatrix(SMatrix& projectionMatrix, float fov, float ar, float n, float f)
{
	projectionMatrix = SMatrix::CreatePerspectiveFieldOfView(fov, ar, n, f);
}

void Math::CreatOrthoMatrix(SMatrix& orthographicMatrix, float w, float h, float n, float f)
{
	orthographicMatrix = SMatrix::CreateOrthographic(w, h, n, f);
}

void Math::CamToViewMatrix(SMatrix& viewMatrix, const SMatrix& cameraMatrix)
{
	viewMatrix = cameraMatrix.Invert();
}

SMatrix Math::CalcMatrixStack(SMatrix& total, const SMatrix& model, const SMatrix& view, const SMatrix& projection)
{
	return model * view * projection;
}

SVec3 Math::getNormalizedVec3(const SVec3& vecToNormalize)
{
	return vecToNormalize / vecToNormalize.Length();
}

SVec3 Math::projectVecOntoVec(const SVec3& toProject, const SVec3& onto)
{
	return (onto.Dot(toProject) / onto.Dot(onto)) * onto;
}

SVec4 Math::fromVec3(SVec3 v, float w)
{
	return SVec4(v.x, v.y, v.z, w);
}

float Math::clamp(float minX, float maxX, float x)
{
	return min(max(minX, x), maxX);
}

float Math::smoothstep(float edge0, float edge1, float x)
{
	float  t = clamp(0.0, 1.0, (x - edge0) / (edge1 - edge0));
	return t * t * (3.0 - 2.0 * t);
}

void Math::swap(float& f1, float& f2)
{
	float temp = f1;
	f1 = f2;
	f2 = temp;
}

float Math::remap(float value, float min1, float max1, float min2, float max2)
{
	float perc = (value - min1) / (max1 - min1);
	return min2 + perc * (max2 - min2);
}