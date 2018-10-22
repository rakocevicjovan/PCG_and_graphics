#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

//#include "Controller.h"
//#include <vector>

class InputManager{

public:
	InputManager();
	~InputManager();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	void SetXY(short, short);

	void GetXY(short& x, short&y);
	bool IsKeyDown(unsigned int);

private:
	bool m_keys[256];
	short curX = 0, curY = 0;

	//std::vector<Controller*> _observers;
};

#endif