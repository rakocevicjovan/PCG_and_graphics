#include "InputManager.h"
#include <assert.h>


InputManager::InputManager()
{
	mouse = std::make_unique<DirectX::Mouse>();
}

InputManager::~InputManager(){}



void InputManager::Initialize()
{

	RAWINPUTDEVICE RIDs[2];	//@TODO do i need this? probably do...

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


void InputManager::KeyDown(unsigned int input)
{
	m_keys[input] = true;
}


void InputManager::KeyUp(unsigned int input)
{
	m_keys[input] = false;
}


void InputManager::SetXY(short x, short y)
{
	curX = x;
	curY = y;
}


void InputManager::GetXY(short& x, short& y) 
{
	x = curX;
	y = curY;
}


bool InputManager::IsKeyDown(unsigned int key)
{
	return m_keys[key];
}