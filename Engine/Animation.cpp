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
		animTransform = getInterpolatedTransform(channel, currentTick, t);	// Animation found, replace local mat
	}
	else
	{
		animTransform = joint._localMatrix;		// No animation channels for this bone right now, use default
	}

	SMatrix nodeTransform = animTransform * parentMatrix;

	// Bind space to bone space, animate, apply global inverse
	SMatrix finalMatrix = joint._offsetMatrix * nodeTransform * glInvT;

	vec[joint.index] = finalMatrix;

	for (Bone* child : joint.offspring)
		getTransformAtTime(*child, vec, nodeTransform, glInvT, elapsed);
}



SMatrix Animation::getInterpolatedTransform(const AnimChannel& channel, float currentTick, float t) const
{
	SVec3 pos = SVec3(0.);
	SVec3 scale = SVec3(1.);
	SQuat quat = SQuat::Identity;

	UINT numPCh = channel.pKeys.size();
	if (numPCh == 1u)
	{
		pos = channel.pKeys[0].pos;
	}
	else
	{
		for (UINT i = 0; i < numPCh; ++i)
		{
			int nextTick = i + 1 == numPCh ? 0 : i + 1;

			if (currentTick < (float)channel.pKeys[nextTick].tick)
			{
				SVec3 posPre = channel.pKeys[i].pos;
				SVec3 posPost = channel.pKeys[nextTick].pos;
				pos = Math::lerp(posPre, posPost, t);
				break;
			}
		}
	}


	UINT numSCh = channel.sKeys.size();
	if (numSCh == 1u)
	{
		scale = channel.sKeys[0].scale;
	}
	else
	{
		for (UINT i = 0; i < numSCh; ++i)
		{
			int nextTick = i + 1 == numSCh ? 0 : i + 1;

			if (currentTick < (float)channel.sKeys[nextTick].tick)
			{
				SVec3 scalePre = channel.sKeys[i].scale;
				SVec3 scalePost = channel.sKeys[nextTick].scale;
				scale = Math::lerp(scalePre, scalePost, t);
				break;
			}
		}
	}


	UINT numRCh = channel.rKeys.size();
	if (numRCh == 1u)
	{
		quat = channel.rKeys[0].rot;
	}
	else
	{
		for (UINT i = 0; i < numRCh; ++i)
		{
			int nextTick = i + 1 == numRCh ? 0 : i + 1;

			if (currentTick < (float)channel.rKeys[nextTick].tick)
			{
				SQuat rotPre = channel.rKeys[i].rot;
				SQuat rotPost = channel.rKeys[nextTick].rot;
				quat = SQuat::Lerp(rotPre, rotPost, t);	// NLERP is faster than SLERP and still good
				break;
			}
		}
	}

	/* Old way, I think slower enough to warrant the change
	SMatrix sMat = SMatrix::CreateScale(scale);
	SMatrix rMat = SMatrix::CreateFromQuaternion(quat);
	SMatrix tMat = SMatrix::CreateTranslation(pos);

	return (sMat * rMat * tMat);	// V * S * R * T
	*/

	return DirectX::XMMatrixAffineTransformation(scale, SVec3(0.f), quat, pos);
}