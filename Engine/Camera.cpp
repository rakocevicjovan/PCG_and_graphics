#include "Camera.h"


Camera::Camera(){
	_controller = nullptr;
}

Camera::Camera(const SMatrix& other) {
	_cameraMatrix = other;
	_viewMatrix = _cameraMatrix.Invert();
}

Camera::~Camera(){
}



/* questionable whether these should exist if controller exists... 
void Camera::SetPosition(SVec3& pos){
	Math::SetTranslation(_cameraMatrix, pos);
}

void Camera::SetRotation(SMatrix& newRotation){
	Math::SetRotation(_cameraMatrix, newRotation);
}


SVec3 Camera::GetPosition() const {
	return _cameraMatrix.Translation();
}

void Camera::setTransform(const SMatrix& transform) {
	_cameraMatrix = transform;
}
/* ...it's not very clean but it can be useful */

SMatrix Camera::GetViewMatrix() const {
	return _viewMatrix;
}

SMatrix Camera::GetCameraMatrix() const {
	return _cameraMatrix;
}

void Camera::SetCameraMatrix(const SMatrix& transform) {
	_cameraMatrix = transform;
}

//@todo add type of update as a short or some other lightweight argument or do something else...
void Camera::update(float dTime) {
	_controller->processTransformationFPS(dTime, _cameraMatrix);
	printf("a");
	_viewMatrix = _cameraMatrix.Invert();
}