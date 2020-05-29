#include "Animation.h"
#include "Bone.h"


Animation::Animation()
{
}


Animation::~Animation()
{
}



void Animation::getTransformAtTime(Bone& joint, std::vector<SMatrix>& vec, const SMatrix& parentMatrix, const SMatrix& glInvT, float elapsed) const
{
	float currentTick = elapsed / _tickDuration;
	float t = fmod(elapsed, _tickDuration) / _tickDuration;

	AnimChannel channel;
	bool found = getAnimChannel(joint.name, channel);

	// This matrix is created according to animation data and current time using interpolation, if no data use default

	SMatrix animTransform;

	if (found)
	{
		animTransform = getInterpolatedTransform(channel, currentTick, t) * joint._localMatrix;	// Animation found, use it
	}
	else
	{
		animTransform = joint._localMatrix;		// No animation channels for this bone right now, use default
	}

	SMatrix nodeTransform = animTransform * parentMatrix;

	// Bind space to bone space, animate, apply global inverse
	SMatrix finalMatrix = joint._offsetMatrix;
	finalMatrix = finalMatrix * nodeTransform;
	finalMatrix = finalMatrix * glInvT;

	// @TODO move responsibility for this to calling code
	//transpose because the shader is column major, nothing to do with the animation process
	vec[joint.index] = finalMatrix.Transpose();

	for (Bone* child : joint.offspring)
		getTransformAtTime(*child, vec, nodeTransform, glInvT, elapsed);
}



SMatrix Animation::getInterpolatedTransform(const AnimChannel& channel, float currentTick, float t) const
{
	SVec3 pos = SVec3(0.);
	SVec3 scale = SVec3(1.);
	SQuat quat = SQuat::Identity;

	for (UINT i = 0; i < channel.pKeys.size() - 1; ++i)
	{
		if (currentTick < (float)channel.pKeys[i + 1].first)
		{
			SVec3 posPre = channel.pKeys[i].second;
			SVec3 posPost = channel.pKeys[i + 1 == channel.pKeys.size() ? 0 : i + 1].second;
			pos = Math::lerp(posPre, posPost, t);
			break;
		}
	}

	for (UINT i = 0; i < channel.sKeys.size() - 1; ++i)
	{
		if (currentTick < (float)channel.sKeys[i + 1].first)
		{
			SVec3 scalePre = channel.sKeys[i].second;
			SVec3 scalePost = channel.sKeys[i + 1 == channel.sKeys.size() ? 0 : i + 1].second;
			scale = Math::lerp(scalePre, scalePost, t);
			break;
		}
	}

	for (UINT i = 0; i < channel.rKeys.size() - 1; ++i)
	{
		if (currentTick < (float)channel.rKeys[i + 1].first)
		{
			SQuat rotPre = channel.rKeys[i].second;
			SQuat rotPost = channel.rKeys[i + 1 == channel.rKeys.size() ? 0 : i + 1].second;
			quat = SQuat::Slerp(rotPre, rotPost, t);	// Could just lerp as well tbh, looks okay
			break;
		}
	}

	SMatrix sMat = SMatrix::CreateScale(scale);
	SMatrix rMat = SMatrix::CreateFromQuaternion(quat);
	SMatrix tMat = SMatrix::CreateTranslation(pos);

	return (sMat * rMat * tMat);	// V * S * R * T
}