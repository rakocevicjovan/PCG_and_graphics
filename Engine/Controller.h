#pragma once
#include "Math.h"

class InputManager;
class CollisionEngine;

class Controller{

private:

	SVec3 processTranslationFPS(const float dTime, const SMatrix& transformation) const;
	void processRotationFPS(const float dTime, SMatrix& transformation) const;
	void applyGravity(const float dTime, SMatrix& transformation) const;
	void resolveCollision(SMatrix& transformation, float dTime, SVec3& velocity);

	float rotCf = 15.f, movCf = 50.f;
	short dx = 0, dy = 0;

	bool _isFlying = true;
	bool _collided = false;
	bool _grounded = false;
	
public:
	Controller();
	Controller(InputManager* inputManager);
	~Controller();

	InputManager* _inMan;
	CollisionEngine* _colEng;
	
	void processTransformationFPS(const float dTime, SMatrix& transformation);
	void toggleFly();
	bool isFlying() { return _isFlying; }
};

