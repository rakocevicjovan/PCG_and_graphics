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
	
protected:
	SMatrix _cameraMatrix;
	SMatrix _viewMatrix;
	SMatrix _projectionMatrix;
};


/* // maybe use this later for different controllers?
class FPSCamera : public Camera {
	FPSCamera();
	FPSCamera(const SMatrix& cameraMatrix);
	~FPSCamera();

	
};



class FreeflyCamera : public Camera {
	FreeflyCamera();
	FreeflyCamera(const SMatrix& cameraMatrix);
	~FreeflyCamera();
};



class OTSCamera : public Camera {
	OTSCamera();
	OTSCamera(const SMatrix& cameraMatrix);
	~OTSCamera();
};

*/
#endif