#include "RTSController.h"


SVec3 RTSController::processTranslation(const float dTime, const SMatrix& transformation) const
{
	return SVec3::Zero;
}



void RTSController::processRotation(float dTime, SMatrix& transformation) const
{
	return;
}



//still can't figure out how to do this... no delta time in event (there SHOULD NOT BE!)
//and I need it for movement... could keep events in a queue but then it's just polling with extra steps
//because the update function would have to poll for existence of events in a queue every step
void RTSController::Observe(const KeyPressMessage& msg)
{
	char key = msg.key;
	bool down = msg.pressed;

	SVec3 delta;

	switch (key)
	{
	case 'W':
		delta.z += 1;
		break;

	case 'A':
		delta.x -= 1;
		break;

	case 'S':
		delta.z -= 1;
		break;

	case 'D':
		delta.z += 1;
		break;
	}

}



void RTSController::Observe(const MouseClickMessage& msg)
{
	short xCoord = msg.x;
	short yCoord = msg.y;
	MBT button = msg.mButtonType;
	bool pressed = msg.pressed;

	switch (button)
	{
	case MBT::MIDDLE:

		break;
	}
}



void update()
{

}