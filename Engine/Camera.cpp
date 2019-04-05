#include "Camera.h"


Camera::Camera(){
	_controller = nullptr;
}

Camera::Camera(const SMatrix& cameraMatrix, const SMatrix& projectionMatrix) {
	_cameraMatrix = cameraMatrix;
	_viewMatrix = _cameraMatrix.Invert();
	_projectionMatrix = projectionMatrix;
}

Camera::~Camera(){
}



SMatrix Camera::GetViewMatrix() const {
	return _viewMatrix;
}

SMatrix Camera::GetCameraMatrix() const {
	return _cameraMatrix;
}

SMatrix Camera::GetProjectionMatrix() const {
	return _projectionMatrix;
}

Camera Camera::CreateFromViewProjection(const SMatrix& view, const SMatrix& projection)
{
	Camera c;
	
	c._viewMatrix = view;
	c._projectionMatrix = projection;
	c._cameraMatrix = view.Invert();
	
	return c;
}

void Camera::SetCameraMatrix(const SMatrix& transform) {
	_cameraMatrix = transform;
}

//@todo add type of update as a short or some other lightweight argument or do something else...
void Camera::update(float dTime)
{
	_controller->processTransformationFPS(dTime, _cameraMatrix);
	_viewMatrix = _cameraMatrix.Invert();
}