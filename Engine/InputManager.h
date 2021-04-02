#pragma once
#include <memory>
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
	std::unique_ptr<DirectX::Mouse> _mouse{};
	DirectX::Mouse::ButtonStateTracker _tracker;

	bool _keys[256]{false};
	MCoords _rel{ 0, 0 };
	MCoords _abs{ 0, 0 };

	void queryMouse();
	void mouseLPressed();
	void mouseLReleased();
	void mouseRPressed();
	void mouseRReleased();

public:

	InputManager();

	void initialize(HWND hwnd);
	void update();

	void registerController(Observer* controller);
	void unregisterController(Observer* controller);

	void setKeyPressed(unsigned int);
	void setKeyReleased(unsigned int);
	
	void getRelativeXY(short& x, short&y);
	void setRelativeXY(short, short);
	
	MCoords getAbsXY() { return _abs; }
	void setAbsXY(const MCoords& abs) { _abs = abs; }

	bool isKeyDown(unsigned int);

	void toggleMouseVisibility();
	bool getMouseVisibility();
};