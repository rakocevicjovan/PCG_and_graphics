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



void Camera::Update(float dTime)
{
	_controller->processTransformationFPS(dTime, _cameraMatrix);
	_viewMatrix = _cameraMatrix.Invert();
}



void Camera::Translate(const SVec3& t)
{
	Math::Translate(_cameraMatrix, t);
	_viewMatrix = _cameraMatrix.Invert();	//@todo optimize this? I assume they can be direcly assigned if one knows how without invert()
}



void Camera::SetTranslation(const SVec3& t)
{


}



void Camera::Rotate(const SMatrix& inRotMat)
{

}



void Camera::Rotate(const SQuat& inQuat)
{

}



void Camera::SetRotation(const SMatrix& inRotMat)
{

}



void Camera::SetRotation(const SQuat& inQuat)
{

}



void Camera::Transform(const SMatrix& inTransform)
{

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