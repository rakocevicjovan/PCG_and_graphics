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
	Controller* _controller{};
	Frustum _frustum;

	Camera() = default;
	Camera(const SMatrix& cameraMatrix, const SMatrix& projectionMatrix);
	Camera(const SMatrix& cameraMatrix, float fov, float ar, float zNear, float zFar);
	~Camera() = default;

	void update(float dTime);

	void translate(const SVec3& t);
	void setPosition(const SVec3& t);

	void rotate(const SMatrix& inRotMat);
	void rotate(const SQuat& inQuat);
	void setRotation(const SMatrix& inRotMat);
	void setRotation(const SQuat& inQuat);

	void transform(const SMatrix& inTransform);
	
	//CTRL_MODE ctrlMode = CTRL_MODE::CTRL_FPS;	void SetMode(CTRL_MODE cm) { ctrlMode = cm; }

	static Camera CreateFromViewProjection(const SMatrix& view, const SMatrix& projection);
	
	const SMatrix& getViewMatrix() const;
	void setViewMatrix(const SMatrix& transform);

	const SMatrix& getCameraMatrix() const;
	void setCameraMatrix(const SMatrix& transform);

	const SMatrix& getProjectionMatrix() const;
	void setProjectionMatrix(const SMatrix& proj);

	SVec3 getPosition() const;
};