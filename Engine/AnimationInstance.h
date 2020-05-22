#pragma once
#include "Animation.h"



class AnimationInstance
{
public:

	const Animation* _anim;
	float _elapsed;



	AnimationInstance(const Animation& anim) : _anim(&anim), _elapsed(0.f) {}



	void update(float dTime)
	{
		_elapsed += dTime;
		_elapsed = fmod(_elapsed, _anim->getDuration());
	}



	void getTransformAtTime(Bone& joint, std::vector<SMatrix>& vec, const SMatrix& parentMatrix, const SMatrix& glInvT)
	{
		float currentTick = _elapsed / _anim->getTickDuration();
		float t = fmod(_elapsed, _anim->getTickDuration());

		AnimChannel channel;
		bool found = _anim->getAnimChannel(joint.name, channel);

		//matrix that is created according to animation data and current time using interpolation, if no data use default
		SMatrix animationMatrix = joint.globalTransform;

		//MATRIX j.aiNodeTransform IS NOT ABSOLUTE TO MESH ROOT, UNLIKE BONE OFFSETS!!! THESE MUST BE CHAINED INTO globalTransform

		SVec3 pos, scale;
		SQuat quat;

		if (found)
		{
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
					quat = SQuat::Slerp(rotPre, rotPost, t);
					break;
				}
			}

			SMatrix sMat = SMatrix::CreateScale(scale);
			SMatrix rMat = SMatrix::CreateFromQuaternion(quat);
			SMatrix tMat = SMatrix::CreateTranslation(pos);

			animationMatrix = sMat * rMat * tMat;
		}

		SMatrix nodeTransform;

		if (found)
			nodeTransform = animationMatrix * joint.locNodeTransform * parentMatrix;
		else
			nodeTransform = animationMatrix * parentMatrix;

		SMatrix finalMatrix =
			joint.meshToLocalBoneSpaceTransform //go from mesh space to bone space
			* nodeTransform						//animate (or default bind if none), then go to parent space (as in, attach the bone to the parent
			* glInvT;							//move the entire mesh to origin

		// @TODO move responsibility for this to calling code
		vec[joint.index] = finalMatrix.Transpose();	//transpose because the shader is column major, nothing to do with the animation process


		for (Bone* child : joint.offspring)
		{
			getTransformAtTime(*child, vec, nodeTransform, glInvT);
		}
	}
};