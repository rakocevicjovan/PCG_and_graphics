#include "pch.h"
#include "Animation.h"
#include "Bone.h"



void Animation::getTransformAtTime(const std::vector<Bone>& bones, BoneIndex boneIndex, const SMatrix& parentMatrix, const SMatrix& glInvT, float elapsed, std::vector<SMatrix>& vec) const
{
	float currentTick = elapsed / _tickDuration;
	float t = currentTick - static_cast<uint32_t>(currentTick);

	const Bone& bone = bones[boneIndex];

	// Optimize this out! It's a temporary fix and awful. 
	// It should be used as a preparation step where each bone is matched to an animation channel once and then has a direct lookup
	const AnimChannel* channel = getAnimChannel(bone._name);

	// If the bone has an animation channel, replace local matrix of the bone with the animated one.
	SMatrix animTransform = channel ? channel->getInterpolatedTransform(currentTick, t) : bone._localMatrix;

	SMatrix nodeTransform = animTransform * parentMatrix;

	// Bind space to bone space, animate, apply global inverse
	SMatrix finalMatrix = bone._invBindPose * nodeTransform * glInvT;

	vec[bone._index] = finalMatrix;

	for (uint16_t childBoneIndex : bone._children)
	{
		getTransformAtTime(bones, childBoneIndex, nodeTransform, glInvT, elapsed, vec);
	}
}