#include "Controller.h"
#include "InputManager.h"


Controller::Controller(){
	inMan = 0;
	xyDelta.dx = 0;
	xyDelta.dy = 0;
}

Controller::Controller(InputManager* inputManager) {
	inMan = inputManager;
}


Controller::~Controller(){
	inMan = nullptr;
}


void Controller::processTransformationFPS(float dTime, SMatrix& transformation) {
	
	xyDelta.old_x = xyDelta.x;
	xyDelta.old_y = xyDelta.y;

	inMan->GetXY(xyDelta.x, xyDelta.y);

	xyDelta.dx = xyDelta.x - xyDelta.old_x;
	xyDelta.dy = xyDelta.y - xyDelta.old_y;
	

	if (!(xyDelta.dx == 0 && xyDelta.dy == 0))	//check if rotation happened, could optimize a lot since this is heavy
		processRotationFPS(dTime, transformation);
	else {
		printf(" ");
	}

	processTranslationFPS(dTime, transformation);
}


void Controller::processRotationFPS(float dTime, SMatrix& transformation) const {

	SMatrix rh;
	rh = rh.CreateFromAxisAngle(SVec3::Up, DirectX::XMConvertToRadians(xyDelta.dx) * rotCf * dTime);

	transformation = transformation * rh;

	SVec3 right =transformation.Right();
	right.Normalize();

	SMatrix rv;
	rv = rv.CreateFromAxisAngle(right, DirectX::XMConvertToRadians(xyDelta.dy) * rotCf * dTime );

	SMatrix rt = rh * rv;

	transformation = transformation * rt;
}



void Controller::processTranslationFPS(const float dTime, SMatrix& transformation) const{

	SVec3 dir = -transformation.Forward();	//this is for rh... fml
	SVec3 right = SVec3::Up.Cross(dir);

	SVec3 deltaTranslation(0, 0, 0);

	if (inMan->IsKeyDown((short)'W')) {
		deltaTranslation = deltaTranslation + dir;
	}

	if (inMan->IsKeyDown((short)'S')) {
		deltaTranslation = deltaTranslation - dir;
	}

	if (inMan->IsKeyDown((short)'A')) {
		deltaTranslation = deltaTranslation - right;
	}

	if (inMan->IsKeyDown((short)'D')) {
		deltaTranslation = deltaTranslation + right;
	}

	if (inMan->IsKeyDown(VK_NUMPAD8)) {
		deltaTranslation = deltaTranslation + SVec3::Up;
	}

	if (inMan->IsKeyDown(VK_NUMPAD2)) {
		deltaTranslation = deltaTranslation - SVec3::Up;
	}

	deltaTranslation.Normalize();

	Math::Translate(transformation, deltaTranslation * dTime);
}
