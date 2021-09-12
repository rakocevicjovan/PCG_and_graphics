#include "pch.h"
#include "Camera.h"


Camera::Camera(const SMatrix& cameraMatrix, const SMatrix& projectionMatrix)
{
	_cameraMatrix = cameraMatrix;
	_viewMatrix = _cameraMatrix.Invert();
	_projectionMatrix = projectionMatrix;

	_frustum = Frustum(projectionMatrix);
}


Camera::Camera(const SMatrix& cameraMatrix, float fov, float ar, float zNear, float zFar)
{
	_cameraMatrix = cameraMatrix;
	_viewMatrix = _cameraMatrix.Invert();
	_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, ar, zNear, zFar);

	_frustum = Frustum(fov, ar, zNear, zFar);
}


Camera Camera::CreateFromViewProjection(const SMatrix& view, const SMatrix& projection)
{
	Camera c;
	
	c._viewMatrix = view;
	c._projectionMatrix = projection;
	c._cameraMatrix = view.Invert();
	c._frustum = Frustum(projection);
	
	return c;
}


void Camera::update(float dTime)
{
	_controller->processTransformationFPS(dTime, _cameraMatrix);
	_viewMatrix = _cameraMatrix.Invert();
	_frustum.update(_viewMatrix * _projectionMatrix);
}


void Camera::transform(const SMatrix& inTransform)
{
	_cameraMatrix *= inTransform;
	_viewMatrix = _cameraMatrix.Invert();
}


void Camera::translate(const SVec3& t)
{
	Math::Translate(_cameraMatrix, t);
	_viewMatrix = _cameraMatrix.Invert();	//@todo can this be optimized to not invert the whole matrix :thinking: probably yes!
}


void Camera::rotate(const SMatrix& inRotMat)
{
	_cameraMatrix *= inRotMat;
	_viewMatrix = _cameraMatrix.Invert();
}


void Camera::rotate(const SQuat& inQuat)
{
	_cameraMatrix = SMatrix::Transform(_cameraMatrix, inQuat);
	_viewMatrix = _cameraMatrix.Invert();
}


const SMatrix& Camera::getViewMatrix() const
{
	return _viewMatrix;
}


void Camera::setViewMatrix(const SMatrix& transform)
{
	_viewMatrix = transform;
	_cameraMatrix = _viewMatrix.Invert();
}


const SMatrix& Camera::getCameraMatrix() const
{
	return _cameraMatrix;
}


void Camera::setCameraMatrix(const SMatrix& transform)
{
	_cameraMatrix = transform;
	_viewMatrix = _cameraMatrix.Invert();
}


const SMatrix& Camera::getProjectionMatrix() const
{
	return _projectionMatrix;
}


void Camera::setProjectionMatrix(const SMatrix& proj)
{
	_projectionMatrix = proj;
	_frustum = Frustum(_projectionMatrix);
}


SVec3 Camera::getPosition() const
{
	return getCameraMatrix().Translation();
}


void Camera::setPosition(const SVec3& t)
{
	Math::SetTranslation(_cameraMatrix, t);
	_viewMatrix = _cameraMatrix.Invert();
}


void Camera::setRotation(const SMatrix& inRotMat)
{
	SVec3 tempTranslation = _cameraMatrix.Translation();
	_cameraMatrix = inRotMat;
	Math::SetTranslation(_cameraMatrix, tempTranslation);
	_viewMatrix = _cameraMatrix.Invert();
}


void Camera::setRotation(const SQuat& inQuat)
{
	SVec3 tempTranslation = _cameraMatrix.Translation();
	_cameraMatrix = SMatrix::CreateFromQuaternion(inQuat);
	Math::SetTranslation(_cameraMatrix, tempTranslation);
	_viewMatrix = _cameraMatrix.Invert();
}