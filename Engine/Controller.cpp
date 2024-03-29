#include "pch.h"
#include "Controller.h"
#include "InputManager.h"


Controller::Controller() : _inMan(nullptr), dx(0), dy(0) {}

Controller::Controller(InputManager* inputManager) : _inMan(inputManager) {}

Controller::~Controller() {}



void Controller::processTransformationFPS(float dTime, SMatrix& transform)
{
	_inMan->getRelativeXY(dx, dy);	//bit lame that I solved it like this, coupled unnecessarily...
	
	if (!(dx == 0 && dy == 0))	// Check if rotation happened, can skip a lot of work
	{
		SVec3 translation = transform.Translation();
		Math::SetTranslation(transform, SVec3());

		processRotationFPS(dTime, transform);

		Math::SetTranslation(transform, translation);
	}
	
	SVec3 velocityVector = processTranslationFPS(dTime, transform) * movCf * dTime;

	Math::Translate(transform, velocityVector);
}



void Controller::processRotationFPS(float dTime, SMatrix& transformation) const
{
	SVec3 snappedUpVector = (transformation.Up().y > 0) ? SVec3(0, 1, 0) : SVec3(0, -1, 0);
	SMatrix rh = SMatrix::CreateFromAxisAngle(snappedUpVector, DirectX::XMConvertToRadians(dx) * rotCf * dTime);
	transformation = transformation * rh;

	SVec3 right = transformation.Right();
	right.Normalize();
	SMatrix rv = SMatrix::CreateFromAxisAngle(right, DirectX::XMConvertToRadians(dy) * rotCf * dTime);
	transformation = transformation * rv;
}



SVec3 Controller::processTranslationFPS(const float dTime, const SMatrix& transformation) const
{
	SVec3 dir = -transformation.Forward();	//this is for rh... fml
	SVec3 right = SVec3::Up.Cross(dir);

	SVec3 deltaTranslation(0, 0, 0);

	if (_inMan->isKeyDown('W')) {
		deltaTranslation = deltaTranslation + dir;
	}

	if (_inMan->isKeyDown('S')) {
		deltaTranslation = deltaTranslation - dir;
	}

	if (_inMan->isKeyDown('A')) {
		deltaTranslation = deltaTranslation - right;
	}

	if (_inMan->isKeyDown('D')) {
		deltaTranslation = deltaTranslation + right;
	}

	if (_inMan->isKeyDown(VK_NUMPAD8)) {
		deltaTranslation = deltaTranslation + SVec3::Up;
	}

	if (_inMan->isKeyDown(VK_NUMPAD2)) {
		deltaTranslation = deltaTranslation - SVec3::Up;
	}

	deltaTranslation.Normalize();

	return deltaTranslation;
}



void Controller::processTransformationTP(float dTime, SMatrix& transform, SMatrix& camTransform)
{
	_inMan->getRelativeXY(dx, dy);

	processRotationTP(dTime, transform, camTransform);

	SVec3 velocityVector = processTranslationTP(dTime, transform, camTransform) * movCf * dTime;

	Math::Translate(transform, velocityVector);
	Math::Translate(camTransform, velocityVector);
}



SVec3 Controller::processTranslationTP(float dTime, const SMatrix & transformation, SMatrix& camTransform) const
{

	SVec3 dir = -transformation.Forward();	//this is for rh... fml
	SVec3 right = transformation.Right();

	SVec3 deltaTranslation(0, 0, 0);

	if (_inMan->isKeyDown('W'))
	{
		deltaTranslation = deltaTranslation + dir;
	}

	if (_inMan->isKeyDown('S'))
	{
		deltaTranslation = deltaTranslation - dir;
	}

	if (_inMan->isKeyDown('Q'))
	{
		deltaTranslation = deltaTranslation - right;
	}

	if (_inMan->isKeyDown('E'))
	{
		deltaTranslation = deltaTranslation + right;
	}

	deltaTranslation.Normalize();

	return deltaTranslation;
}



void Controller::processRotationTP(float dTime, SMatrix& transform, SMatrix& camTransform) const
{
	//old translation of the player
	SVec3 tempTranslation = transform.Translation();
	Math::SetTranslation(transform, SVec3());

	float rotator = 0;

	if (_inMan->isKeyDown('A'))
		rotator += rotCf * .1f * dTime;

	if (_inMan->isKeyDown('D'))
		rotator -= rotCf * .1f * dTime;

	transform *= SMatrix::CreateRotationY(-rotator);
	Math::SetTranslation(transform, tempTranslation);

	//camera rotation around player
	SVec3 playerToCam = Math::getNormalizedVec3(camTransform.Translation() - transform.Translation());

	SMatrix rh = SMatrix::CreateFromAxisAngle(SVec3::Up, DirectX::XMConvertToRadians(dx) * rotCf * dTime);
	Math::RotateVecByMat(playerToCam, rh);
	
	SMatrix rv = SMatrix::CreateFromAxisAngle(transform.Right(), DirectX::XMConvertToRadians(dy) * rotCf * dTime);
	Math::RotateVecByMat(playerToCam, rv);

	Math::SetTranslation(camTransform, transform.Translation() + playerToCam * camDist);

	camTransform = SMatrix::CreateWorld(camTransform.Translation(), playerToCam, SVec3(0, 1, 0));
}



void Controller::Observe(const KeyPressMessage& msg)
{
	switch (msg.key)
	{
	case 'F':
		if (msg.pressed) toggleFlying();	//every press down toggle, so I don't have to hold it
		break;

	case 'M':
		if (msg.pressed)
		{
			_showCursor = !_showCursor;
			ShowCursor(_showCursor);
		}
		break;

	default:
		break;
	}
}

void Controller::mouseLPressed(const MCoords& absmc)
{
	return;
}


void Controller::mouseLReleased(const MCoords& absmc)
{
	return;
}


void Controller::mouseRPressed(const MCoords& absmc)
{
	return;
}


void Controller::mouseRReleased(const MCoords& absmc)
{
	return;
}