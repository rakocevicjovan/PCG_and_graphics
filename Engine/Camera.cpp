#include "Camera.h"


Camera::Camera()
{
	_controller = nullptr;
}



Camera::Camera(const SMatrix& cameraMatrix, const SMatrix& projectionMatrix)
{
	_cameraMatrix = cameraMatrix;
	_viewMatrix = _cameraMatrix.Invert();
	_projectionMatrix = projectionMatrix;

	frustum = Frustum(projectionMatrix);
}



Camera::~Camera() {}



Camera Camera::CreateFromViewProjection(const SMatrix& view, const SMatrix& projection)
{
	Camera c;
	
	c._viewMatrix = view;
	c._projectionMatrix = projection;
	c._cameraMatrix = view.Invert();
	
	return c;
}



void Camera::SetCameraMatrix(const SMatrix& transform)
{
	_cameraMatrix = transform;
	_viewMatrix = _cameraMatrix.Invert();
}



void Camera::SetProjectionMatrix(const SMatrix& proj)
{
	_projectionMatrix = proj;
	frustum = Frustum(_projectionMatrix);
}



void Camera::Update(float dTime)
{
	_controller->processTransformationFPS(dTime, _cameraMatrix);
	_viewMatrix = _cameraMatrix.Invert();
	frustum.update(_viewMatrix * _projectionMatrix);
}



void Camera::Translate(const SVec3& t)
{
	Math::Translate(_cameraMatrix, t);
	_viewMatrix = _cameraMatrix.Invert();	//@todo can this be optimized to not invert the whole matrix :thinking: probably yes!
}



void Camera::SetTranslation(const SVec3& t)
{
	Math::SetTranslation(_cameraMatrix, t);
	_viewMatrix = _cameraMatrix.Invert();
}



void Camera::Rotate(const SMatrix& inRotMat)
{
	_cameraMatrix *= inRotMat;
	_viewMatrix = _cameraMatrix.Invert();
}



void Camera::Rotate(const SQuat& inQuat)
{
	_cameraMatrix = SMatrix::Transform(_cameraMatrix, inQuat);
	_viewMatrix = _cameraMatrix.Invert();
}



void Camera::SetRotation(const SMatrix& inRotMat)
{
	SVec3 tempTranslation = _cameraMatrix.Translation();
	_cameraMatrix = inRotMat;
	Math::SetTranslation(_cameraMatrix, tempTranslation);
	_viewMatrix = _cameraMatrix.Invert();
}



void Camera::SetRotation(const SQuat& inQuat)
{
	SVec3 tempTranslation = _cameraMatrix.Translation();
	_cameraMatrix = SMatrix::CreateFromQuaternion(inQuat);
	Math::SetTranslation(_cameraMatrix, tempTranslation);
	_viewMatrix = _cameraMatrix.Invert();
}



void Camera::Transform(const SMatrix& inTransform)
{
	_cameraMatrix *= inTransform;
	_viewMatrix = _cameraMatrix.Invert();
}



SMatrix Camera::GetViewMatrix() const
{
	return _viewMatrix;
}



SMatrix Camera::GetCameraMatrix() const
{
	return _cameraMatrix;
}



SMatrix Camera::GetProjectionMatrix() const
{
	return _projectionMatrix;
}



SVec3 Camera::GetPosition() const
{
	return GetCameraMatrix().Translation();
}
