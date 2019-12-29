#include "InputManager.h"
#include <assert.h>
#include <algorithm>



InputManager::InputManager()
{
	_mouse = std::make_unique<DirectX::Mouse>();
}


InputManager::~InputManager()
{}


void InputManager::Initialize(HWND hwnd)
{
	_mouse->SetWindow(hwnd);

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
		_keys[i] = false;
}


void InputManager::registerController(Observer* controller)
{
	_observers.push_back(controller);
}


void InputManager::unregisterController(Observer* controller)
{
	_observers.erase(std::remove(_observers.begin(), _observers.end(), controller), _observers.end());
}


void InputManager::setKeyPressed(unsigned int input)
{
	_keys[input] = true;

	for (auto obs : _observers)
		obs->Observe(KeyPressMessage((char)input, true ));
}


void InputManager::setKeyReleased(unsigned int input)
{
	_keys[input] = false;

	for (auto obs : _observers)
		obs->Observe(KeyPressMessage((char)input, false));
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
	return _keys[key];
}


void InputManager::queryMouse()
{
	DirectX::Mouse::State state = _mouse->GetState();
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
		obs->Observe({_abs.x, _abs.y, MBT::LEFT, 1});
}


void InputManager::mouseLReleased()
{
	for (auto obs : _observers)
		obs->Observe(MouseClickMessage(_abs.x, _abs.y, MBT::LEFT, 0));
}


void InputManager::mouseRPressed()
{
	for (auto obs : _observers)
		obs->Observe({ _abs.x, _abs.y, MBT::RIGHT, 1 });
	return;
}


void InputManager::mouseRReleased()
{
	for (auto obs : _observers)
		obs->Observe({ _abs.x, _abs.y, MBT::RIGHT, 0 });
	return;
}


void InputManager::toggleMouseMode()
{
	_cursorVisible = !_cursorVisible;
	_mouse->SetVisible(_cursorVisible);
}


bool InputManager::getMouseMode()
{
	return _cursorVisible;
}