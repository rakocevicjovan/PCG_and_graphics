#pragma once
#include "Math.h"

class InputManager;

class Controller{

private:

	void processTranslationFPS(const float dTime, SMatrix& transformation) const;
	void processRotationFPS(const float dTime, SMatrix& transformation) const;

public:
	Controller();
	Controller(InputManager* inputManager);
	~Controller();

	InputManager* inMan;
	float rotCf = 15.f , movCf = 100.f;
	short dx = 0, dy = 0;

	void processTransformationFPS(const float dTime, SMatrix& transformation);
};

