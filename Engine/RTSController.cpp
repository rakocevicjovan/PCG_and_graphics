#include "RTSController.h"


SVec3 RTSController::processTranslation(const float dTime, const SMatrix& transformation) const
{

}



void RTSController::processRotation(float dTime, SMatrix& transformation) const
{

}



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