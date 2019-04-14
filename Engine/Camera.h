#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include "Math.h"
#include "Controller.h"

class Camera{

public:
	Camera();
	Camera(const SMatrix& cameraMatrix, const SMatrix& projectionMatrix);
	~Camera();

	void Update(float dTime);
	void SetCameraMatrix(const SMatrix& transform);

	void Translate(const SVec3& t);
	void SetTranslation(const SVec3& t);

	void Rotate(const SMatrix& inRotMat);
	void Rotate(const SQuat& inQuat);
	void SetRotation(const SMatrix& inRotMat);
	void SetRotation(const SQuat& inQuat);

	void Transform(const SMatrix& inTransform);

	Controller* _controller;

	static Camera CreateFromViewProjection(const SMatrix& view, const SMatrix& projection);
	SMatrix GetViewMatrix() const;
	SMatrix GetCameraMatrix() const;
	SMatrix GetProjectionMatrix() const;
	SVec3   GetPosition() const;
	
protected:
	SMatrix _cameraMatrix;
	SMatrix _viewMatrix;
	SMatrix _projectionMatrix;
};
#endif