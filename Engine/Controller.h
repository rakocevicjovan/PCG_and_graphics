#pragma once
#include "Math.h"

class InputManager;

class Controller{

private:

	void processTranslationFPS(const float dTime, SMatrix& transformation) const;
	void processRotationFPS(const float dTime, SMatrix& transformation) const;
	void applyGravity(const float dTime, SMatrix& transformation) const;
	void setCollisionOffset(const SVec3& collisionOffset);
	void resolveCollision(SMatrix& transformation);

	float rotCf = 15.f, movCf = 100.f;
	short dx = 0, dy = 0;

	bool _isFlying = true;
	bool _collided = false;

	SVec3 _collisionOffset;
	

public:
	Controller();
	Controller(InputManager* inputManager);
	~Controller();

	InputManager* inMan;
	
	void processTransformationFPS(const float dTime, SMatrix& transformation);
	void toggleFly();
};

