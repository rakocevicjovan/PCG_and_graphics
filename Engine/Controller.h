#pragma once
#include "Math.h"

class InputManager;

class Controller{

public:
	Controller();
	Controller::Controller(InputManager* inputManager);
	~Controller();

	InputManager* inMan;
	float rotCf = 15.f , movCf = 100.f;
	short dx = 0, dy = 0;

	void Controller::processTransformationFPS(const float dTime, SMatrix& transformation);
	void Controller::processTranslationFPS(const float dTime, SMatrix& transformation) const;
	void Controller::processRotationFPS(const float dTime, SMatrix& transformation) const;

};

