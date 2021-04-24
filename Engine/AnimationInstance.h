#pragma once
#include "Animation.h"



class AnimationInstance
{
public:

	const Animation* _anim;
	float _elapsed;



	AnimationInstance(const Animation* anim) : _anim(anim), _elapsed(0.f) {}



	inline void update(float dTime)
	{
		_elapsed += dTime;
		_elapsed = fmod(_elapsed, _anim->getDuration());
	}



	inline void getTransformAtTime(const std::vector<Bone>& bones, uint16_t boneIndex, std::vector<SMatrix>& vec, const SMatrix& parentMatrix, const SMatrix& glInvT)
	{
		_anim->getTransformAtTime(bones, boneIndex, parentMatrix, glInvT, _elapsed, vec);
	}
};