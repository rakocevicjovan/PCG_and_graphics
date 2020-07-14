#include "Animation.h"
#include "Bone.h"



void Animation::getTransformAtTime(Bone& bone, std::vector<SMatrix>& vec, const SMatrix& parentMatrix, const SMatrix& glInvT, float elapsed) const
{
	float currentTick = elapsed / _tickDuration;
	float t = currentTick - (long)currentTick;
	
	const AnimChannel* channel = getAnimChannel(bone._name);

	SMatrix animTransform;

	if (channel)
		animTransform = channel->getInterpolatedTransform(currentTick, t);	// Animation found, replace local mat
	else
		animTransform = bone._localMatrix;		// No animation channels for this bone right now, use default

	SMatrix nodeTransform = animTransform * parentMatrix;

	// Bind space to bone space, animate, apply global inverse
	SMatrix finalMatrix = bone._offsetMatrix * nodeTransform * glInvT;

	vec[bone._index] = finalMatrix;

	for (Bone* child : bone.offspring)
		getTransformAtTime(*child, vec, nodeTransform, glInvT, elapsed);
}