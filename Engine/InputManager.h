#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#include <memory>
#include <Windows.h>
#include "Mouse.h"

class InputManager{

public:

	InputManager();
	~InputManager();

	void Initialize(HWND hwnd);

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	void SetXY(short, short);

	void GetXY(short& x, short&y);
	bool IsKeyDown(unsigned int);

	void ToggleMouseMode();
	bool GetMouseMode();

	std::unique_ptr<DirectX::Mouse> mouse;

private:
	bool m_keys[256];
	bool cursorVisible = false;
	short curX = 0, curY = 0;
};

#endif