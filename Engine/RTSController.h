#pragma once
#include "Observer.h"
#include "InputManager.h"
#include "Math.h"

class RTSController : public Observer
{
private:
	InputManager* _inMan;

	float _rotationCoefficient = 15.f;
	float _movementCoefficient = 50.f;

	SVec3 processTranslation(const float dTime, const SMatrix& transformation) const;
	void processRotation(float dTime, SMatrix& transformation) const;

public:
	void Observe(const KeyPressMessage& msg) override;
	void Observe(const MouseClickMessage& msg) override;
};