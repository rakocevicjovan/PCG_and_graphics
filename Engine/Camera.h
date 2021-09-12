#pragma once

#include "Controller.h"
#include "Frustum.h"



class Camera
{
protected:
	SMatrix _cameraMatrix;
	SMatrix _viewMatrix;
	SMatrix _projectionMatrix;

public:
	Controller* _controller;
	Frustum _frustum;

	Camera();
	Camera(const SMatrix& cameraMatrix, const SMatrix& projectionMatrix);
	Camera(const SMatrix& cameraMatrix, float fov, float ar, float zNear, float zFar);
	~Camera();

	void Update(float dTime);

	void Translate(const SVec3& t);
	void SetTranslation(const SVec3& t);

	void Rotate(const SMatrix& inRotMat);
	void Rotate(const SQuat& inQuat);
	void SetRotation(const SMatrix& inRotMat);
	void SetRotation(const SQuat& inQuat);

	void Transform(const SMatrix& inTransform);
	
	//CTRL_MODE ctrlMode = CTRL_MODE::CTRL_FPS;	void SetMode(CTRL_MODE cm) { ctrlMode = cm; }

	static Camera CreateFromViewProjection(const SMatrix& view, const SMatrix& projection);
	SMatrix GetViewMatrix() const;

	SMatrix GetCameraMatrix() const;
	void SetCameraMatrix(const SMatrix& transform);

	SMatrix GetProjectionMatrix() const;
	void SetProjectionMatrix(const SMatrix& proj);

	SVec3   GetPosition() const;
};