#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include "Math.h"
#include "Controller.h"

class Camera{

public:
	Camera();
	Camera(const SMatrix& cameraMatrix, const SMatrix& projectionMatrix);
	~Camera();

	void update(float dTime);
	void SetCameraMatrix(const SMatrix& transform);

	SMatrix GetViewMatrix() const;
	SMatrix GetCameraMatrix() const;
	SMatrix GetProjectionMatrix() const;
	Controller* _controller;

	static Camera CreateFromViewProjection(const SMatrix& view, const SMatrix& projection);
	
protected:
	SMatrix _cameraMatrix;
	SMatrix _viewMatrix;
	SMatrix _projectionMatrix;
};
#endif