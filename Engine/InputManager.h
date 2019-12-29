#pragma once
#include <memory>
#include <Windows.h>
#include <vector>
#include "Mouse.h"
#include "Controller.h"
#include "Observer.h"

struct MCoords
{
	short x;
	short y;

	MCoords(short x, short y) : x(x), y(y) {}
	MCoords() : x(0), y(0) {}
};


class InputManager
{
private:
	std::vector<Observer*> _observers;
	std::unique_ptr<DirectX::Mouse> _mouse;
	DirectX::Mouse::ButtonStateTracker tracker;

	bool _keys[256];
	bool _cursorVisible = false;
	MCoords _rel;
	MCoords _abs;

public:

	InputManager();
	~InputManager();

	void Initialize(HWND hwnd);

	void registerController(Observer* controller);
	void unregisterController(Observer* controller);

	void setKeyPressed(unsigned int);
	void setKeyReleased(unsigned int);
	
	void getRelativeXY(short& x, short&y);
	void setRelativeXY(short, short);
	
	MCoords getAbsXY() { return _abs; }
	void setAbsXY(const MCoords& abs) { _abs = abs; }

	bool isKeyDown(unsigned int);

	void toggleMouseMode();
	bool getMouseMode();

	void queryMouse();
	void mouseLPressed();
	void mouseLReleased();
	void mouseRPressed();
	void mouseRReleased();
};