#pragma once
#include "Mouse.h"
#include "Controller.h"
#include "Observer.h"


struct MCoords
{
	int16_t x{0u};
	int16_t y{0u};
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

	void setKeyPressed(uint32_t);
	void setKeyReleased(uint32_t);
	
	void getRelativeXY(int16_t& x, int16_t&y);
	void setRelativeXY(int16_t, int16_t);
	
	MCoords getAbsXY() { return _abs; }
	void setAbsXY(const MCoords& abs) { _abs = abs; }

	bool isKeyDown(uint32_t);

	void toggleMouseVisibility();
	bool getMouseVisibility();
};