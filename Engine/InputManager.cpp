#include "pch.h"
#include "InputManager.h"



InputManager::InputManager() : _mouse(std::make_unique<DirectX::Mouse>())
{}


void InputManager::initialize(HWND hwnd)
{
	_mouse->SetWindow(hwnd);

	// See https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinputdevice
	RAWINPUTDEVICE RIDs[2];			//@TODO do i need this? probably do...

	RIDs[0].usUsagePage = 0x01;		// Top level collection - keyboard and mouse. There can be multiple of these.
	RIDs[0].usUsage = 0x02;			// Code of our device, 0x02 is mouse, 0x06 is keyboard, others are possible too
	RIDs[0].dwFlags = 0;			// default way to interpret the device
	RIDs[0].hwndTarget = NULL;		// follows the keyboard focus

	RIDs[1].usUsagePage = 0x01;
	RIDs[1].usUsage = 0x06;
	RIDs[1].dwFlags = 0;
	RIDs[1].hwndTarget = NULL;

	if (RegisterRawInputDevices(RIDs, 2, sizeof(RIDs[0])) == false)
	{
		OutputDebugStringA("Could not register the raw input devices.");
		__debugbreak();
	}

	for(int i=0; i<256; i++)
		_keys[i] = false;
}


void InputManager::update()
{
	queryMouse();
	setRelativeXY(0, 0);
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
		obs->Observe(KeyPressMessage((char)input, true));
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
	_tracker.Update(state);
	_abs.x = state.x;
	_abs.y = state.y;

	if (_tracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
	{
		mouseLPressed();
	}
	if (_tracker.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED)
	{
		mouseLReleased();
	}
	if (_tracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
	{
		mouseRPressed();
	}
	if (_tracker.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED)
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
}


void InputManager::mouseRReleased()
{
	for (auto obs : _observers)
		obs->Observe({ _abs.x, _abs.y, MBT::RIGHT, 0 });
}


void InputManager::toggleMouseVisibility()
{
	_mouse->SetVisible(!_mouse->IsVisible());
}


bool InputManager::getMouseVisibility()
{
	return _mouse->IsVisible();
}