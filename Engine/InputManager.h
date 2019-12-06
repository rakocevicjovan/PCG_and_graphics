#pragma once
#include <memory>
#include <Windows.h>
#include <vector>
#include "Mouse.h"
#include "Controller.h"

class InputManager
{
private:
	std::vector<Controller*> _observers;
	std::unique_ptr<DirectX::Mouse> mouse;
	DirectX::Mouse::ButtonStateTracker tracker;

	bool m_keys[256];
	bool cursorVisible = false;
	short curX = 0, curY = 0;
public:

	InputManager();
	~InputManager();

	void Initialize(HWND hwnd);

	void registerController(Controller* controller);
	void unregisterController(Controller* controller);

	void setKeyPressed(unsigned int);
	void setKeyReleased(unsigned int);
	
	void setRelativeXY(short, short);
	void getRelativeXY(short& x, short&y);

	bool isKeyDown(unsigned int);

	void toggleMouseMode();
	bool getMouseMode();

	void queryMouse();
	void mouseLPressed();
	void mouseLReleased();
	void mouseRPressed();
	void mouseRReleased();
};