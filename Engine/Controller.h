#pragma once
#include "Math.h"

class InputManager;

class Controller{

	struct DeltaCoords {
		
		short x = 0;
		short y = 0;
		short old_x = 0;
		short old_y = 0;
		short dx = 0;
		short dy = 0;

		DeltaCoords(){}
	} xyDelta;

public:
	Controller();
	Controller::Controller(InputManager* inputManager);
	~Controller();

	InputManager* inMan;
	float rotCf = 0.5f , movCf = 0.5f;

	void Controller::processTransformationFPS(const float dTime, SMatrix& transformation);
	void Controller::processTranslationFPS(const float dTime, SMatrix& transformation) const;
	void Controller::processRotationFPS(const float dTime, SMatrix& transformation) const;

};

