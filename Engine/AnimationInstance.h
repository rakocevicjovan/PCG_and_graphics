#pragma once
#include "Animation.h"



class AnimationInstance
{
private:

	std::shared_ptr<Animation> _anim{};
	float _elapsed{ 0.f };

public:

	AnimationInstance(std::shared_ptr<Animation> anim) : _anim(anim) {}


	inline void update(float dTime)
	{
		auto animDuration = _anim->getDuration();

		_elapsed += dTime;
		_elapsed = fmod(animDuration + _elapsed,  animDuration);	// Allow reversed animations
	}


	inline void getTransformAtTime(const std::vector<Bone>& bones, std::vector<SMatrix>& vec, const SMatrix& glInvT)
	{
		_anim->getTransformAtTime(bones, glInvT, _elapsed, vec);
	}

	inline float elapsed() { return _elapsed; }
	inline void restart() { _elapsed = 0.f; }
};