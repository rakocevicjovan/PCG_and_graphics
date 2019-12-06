#pragma once
#include "Math.h"

class InputManager;
class CollisionEngine;


enum CTRL_MODE
{
	CTRL_FPS,
	CTRL_TP,
	CTRL_FREEFLY
};


class Controller
{
private:
	InputManager* _inMan;

	SVec3 processTranslationFPS(const float dTime, const SMatrix& transformation) const;
	void processRotationFPS(float dTime, SMatrix& transformation) const;

	SVec3 processTranslationTP(float dTime, const SMatrix& transformation, SMatrix& camTransformation) const;
	void processRotationTP(float dTime, SMatrix& transformation, SMatrix& camTransformation) const;

	void applyGravity(const float dTime, SMatrix& transformation) const;
	void resolveCollision(SMatrix& transformation, float dTime, SVec3& velocity);

	float rotCf = 15.f, movCf = 50.f, camDist = 66.f;
	short dx = 0, dy = 0;

	bool _isFlying = true;
	bool _collided = false;
	bool _grounded = false;
	bool _showCursor = false;
	
public:
	Controller();
	Controller(InputManager* inputManager);
	~Controller();

	CollisionEngine* _colEng;	//this should not be here by any means!!!
	
	void processTransformationFPS(float dTime, SMatrix& transformation);
	void processTransformationTP(float dTime, SMatrix& transform, SMatrix& camTransform);

	void processCommonInputs(float dTime);
	void notify(char key, bool pressed);
	void toggleFlying();
	void setFlying(bool b);
	bool isFlying() { return _isFlying; }

	void mouseLPressed();
	void mouseLReleased();
	void mouseRPressed();
	void mouseRReleased();
};