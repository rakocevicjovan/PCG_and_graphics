#include "pch.h"
#include "Animation.h"
#include "Bone.h"



void Animation::getTransformAtTime(const Bone& bone, const SMatrix& parentMatrix, const SMatrix& glInvT, float elapsed, std::vector<SMatrix>& vec) const
{
	float currentTick = elapsed / _tickDuration;
	float t = currentTick - static_cast<uint32_t>(currentTick);

	const AnimChannel* channel = getAnimChannel(bone._name);

	SMatrix animTransform;

	if (channel)	// Animation found, replace local mattrix
		animTransform = channel->getInterpolatedTransform(currentTick, t);	
	else			// No animation channels found, use local matrix
		animTransform = bone._localMatrix;

	SMatrix nodeTransform = animTransform * parentMatrix;

	// Bind space to bone space, animate, apply global inverse
	SMatrix finalMatrix = bone._offsetMatrix * nodeTransform * glInvT;

	vec[bone._index] = finalMatrix;

	for (Bone* child : bone._children)
		getTransformAtTime(*child, nodeTransform, glInvT, elapsed, vec);
}