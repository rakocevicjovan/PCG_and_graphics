#include "pch.h"
#include "Animation.h"
#include "Bone.h"



Animation::Animation() noexcept = default;


Animation::Animation(std::string& name, double ticks, double ticksPerSecond, int nc) noexcept
	: _name(name), _ticks(ticks), _ticksPerSecond(ticksPerSecond)
{
	_duration = _ticks / _ticksPerSecond;
	_tickDuration = _duration / _ticks;
}


void Animation::getTransformAtTime(const std::vector<Bone>& bones, const SMatrix& glInvT, float elapsed, std::vector<SMatrix>& vec) const
{
	float currentTick = elapsed / _tickDuration;
	float t = currentTick - static_cast<uint32_t>(currentTick);

	for (auto i = 0; i < bones.size(); ++i)
	{
		const Bone& bone = bones[i];

		SMatrix parentMatrix;
		
		[[likely]]
		if (bone._parent != Bone::INVALID_INDEX)
		{
			parentMatrix = vec[bone._parent];
		}

		// Optimize this out! It's a temporary fix and awful. 
		// It should be used as a preparation step where each bone is matched to an animation channel once and then has a direct lookup
		const AnimChannel* channel = getAnimChannel(getAnimChannelIndex(bone.name()));

		// If the bone has an animation channel, replace local matrix of the bone with the animated one.
		SMatrix animTransform = channel ? channel->getInterpolatedTransform(currentTick, t) : bone._localMatrix;

		vec[i] = animTransform * parentMatrix;
	}

	// Bind space to bone space, animate, apply global inverse - but only for influence bones
	uint32_t realIndex{ 0u };

	for (auto i = 0; i < bones.size(); ++i)
	{
		if (bones[i]._isInfluenceBone)
		{
			vec[realIndex++] = bones[i]._invBindPose * vec[i] * glInvT;
		}
	}
}