#include "Controller.h"
#include "InputManager.h"
#include "CollisionEngine.h"


Controller::Controller() : _inMan(nullptr), dx(0), dy(0) {}

Controller::Controller(InputManager* inputManager) : _inMan(inputManager) {}

Controller::~Controller(){}



void Controller::processTransformationFPS(float dTime, SMatrix& transformation)
{
	_inMan->GetXY(dx, dy);

	SVec3 translation = transformation.Translation();
	Math::SetTranslation(transformation, SVec3());

	if (!(dx == 0 && dy == 0))	//check if rotation happened, can skip a lot of work
		processRotationFPS(dTime, transformation);

	Math::SetTranslation(transformation, translation);
	SVec3 velocityVector = processTranslationFPS(dTime, transformation) * movCf * dTime;
	
	//flying mode is used for testing and shouldn't collide or fall for convenience
	if (!_isFlying)
	{
		resolveCollision(transformation, dTime, velocityVector);	//change velocity vector first if affected by collision
		applyGravity(dTime, transformation);
	}

	Math::Translate(transformation, velocityVector);
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



SVec3 Controller::processTranslationFPS(const float dTime, const SMatrix& transformation) const{

	SVec3 dir = -transformation.Forward();	//this is for rh... fml
	SVec3 right = SVec3::Up.Cross(dir);

	SVec3 deltaTranslation(0, 0, 0);

	if (_inMan->IsKeyDown((short)'W')) {
		deltaTranslation = deltaTranslation + dir;
	}

	if (_inMan->IsKeyDown((short)'S')) {
		deltaTranslation = deltaTranslation - dir;
	}

	if (_inMan->IsKeyDown((short)'A')) {
		deltaTranslation = deltaTranslation - right;
	}

	if (_inMan->IsKeyDown((short)'D')) {
		deltaTranslation = deltaTranslation + right;
	}

	if (_inMan->IsKeyDown(VK_NUMPAD8)) {
		deltaTranslation = deltaTranslation + SVec3::Up;
	}

	if (_inMan->IsKeyDown(VK_NUMPAD2)) {
		deltaTranslation = deltaTranslation - SVec3::Up;
	}

	deltaTranslation.Normalize();

	return deltaTranslation;
}



void Controller::applyGravity(const float dTime, SMatrix& transformation) const
{
	if(!_grounded)
		Math::Translate(transformation, SVec3(0.f, -9.81f, 0.f) * dTime);
}



void Controller::toggleFly()
{
	_isFlying = _isFlying ? false : true;
}



void Controller::resolveCollision(SMatrix& transformation, float dTime, SVec3& velocity)
{
	if (!_colEng) return;

	SVec3 collisionOffset = _colEng->resolvePlayerCollision(transformation, velocity);

	if (collisionOffset.LengthSquared() > .1f)
		_grounded = true;
	else
		_grounded = false;
}