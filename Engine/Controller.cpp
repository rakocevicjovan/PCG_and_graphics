#include "Controller.h"
#include "InputManager.h"


Controller::Controller(){
	inMan = 0;
	dx = 0;
	dy = 0;
}

Controller::Controller(InputManager* inputManager) 
{
	inMan = inputManager;
}


Controller::~Controller()
{
	inMan = nullptr;
}


void Controller::processTransformationFPS(float dTime, SMatrix& transformation) {

	inMan->GetXY(dx, dy);

	SVec3 translation = transformation.Translation();
	Math::SetTranslation(transformation, SVec3());

	if (!(dx == 0 && dy == 0))	//check if rotation happened, could optimize a lot since this is heavy
		processRotationFPS(dTime, transformation);
	else {
		printf(" ");
	}

	Math::SetTranslation(transformation, translation);
	processTranslationFPS(dTime, transformation);
}


void Controller::processRotationFPS(float dTime, SMatrix& transformation) const {

	SMatrix rh;
	rh = rh.CreateFromAxisAngle(SVec3::Up, DirectX::XMConvertToRadians(dx) * rotCf * dTime);

	transformation = transformation * rh;

	SVec3 right = transformation.Right();
	right.Normalize();

	SMatrix rv;
	rv = rv.CreateFromAxisAngle(right, DirectX::XMConvertToRadians(dy) * rotCf * dTime );

	transformation = transformation * rv ;
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

	Math::Translate(transformation, deltaTranslation * movCf * dTime);
}