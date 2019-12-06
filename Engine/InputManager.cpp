#include "InputManager.h"
#include <assert.h>
#include <algorithm>



InputManager::InputManager()
{
	mouse = std::make_unique<DirectX::Mouse>();
}



InputManager::~InputManager(){}



void InputManager::Initialize(HWND hwnd)
{
	mouse->SetWindow(hwnd);

	RAWINPUTDEVICE RIDs[2];			//@TODO do i need this? probably do...

	RIDs[0].usUsagePage = 0x01;		//keyboard and mouse
	RIDs[0].usUsage = 0x02;			//code of our device, 0x06 is keyboard, 0x02 is mouse... could use 2 and 6 too but this makes me look smarter
	RIDs[0].dwFlags = 0;			//default way to interpret the device
	RIDs[0].hwndTarget = NULL;		//follows the keyboard focus

	RIDs[1].usUsagePage = 0x01;
	RIDs[1].usUsage = 0x06;
	RIDs[1].dwFlags = 0;
	RIDs[1].hwndTarget = NULL;

	if (RegisterRawInputDevices(RIDs, 2, sizeof(RIDs[0])) == false)
		MessageBoxW(NULL, L"Could not register the raw input devices", L"Raw input fail", 0);

	for(int i=0; i<256; i++)
		m_keys[i] = false;
}


void InputManager::registerController(Controller* controller)
{
	_observers.push_back(controller);
}


void InputManager::unregisterController(Controller* controller)
{
	_observers.erase(std::remove(_observers.begin(), _observers.end(), controller), _observers.end());
}


void InputManager::setKeyPressed(unsigned int input)
{
	m_keys[input] = true;

	for (auto obs : _observers)
		obs->notify((char)input, true);
}


void InputManager::setKeyReleased(unsigned int input)
{
	m_keys[input] = false;

	for (auto obs : _observers)
		obs->notify((char)input, false);
}


void InputManager::setRelativeXY(short x, short y)
{
	_rel.x = x;
	_rel.y = y;
}


void InputManager::getRelativeXY(short& x, short& y) 
{
	x = _rel.x;
	y = _rel.y;
}


bool InputManager::isKeyDown(unsigned int key)
{
	return m_keys[key];
}


void InputManager::queryMouse()
{
	DirectX::Mouse::State state = mouse->GetState();
	tracker.Update(state);
	_abs.x = state.x;
	_abs.y = state.y;

	if (tracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
	{
		mouseLPressed();
	}
	if (tracker.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED)
	{
		mouseLReleased();
	}
	if (tracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
	{
		mouseRPressed();
	}
	if (tracker.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED)
	{
		mouseRReleased();
	}
}


void InputManager::mouseLPressed()
{
	for (auto obs : _observers)
		obs->mouseLPressed(_abs);
}


void InputManager::mouseLReleased()
{
	for (auto obs : _observers)
		obs->mouseLReleased(_abs);
}


void InputManager::mouseRPressed()
{
	return;
}


void InputManager::mouseRReleased()
{
	return;
}


void InputManager::toggleMouseMode()
{
	cursorVisible = !cursorVisible;
	mouse->SetVisible(cursorVisible);
}


bool InputManager::getMouseMode()
{
	return cursorVisible;
}