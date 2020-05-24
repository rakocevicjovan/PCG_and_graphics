#pragma once
#include "Math.h"
#include "MeshDataStructs.h"
#include <string>
#include <vector>
#include <map>


struct AnimChannel
{
	AnimChannel() {}

	AnimChannel(int p, int r, int s)
	{
		pKeys.reserve(p);
		rKeys.reserve(r);
		sKeys.reserve(s);
	}

	std::string jointName;

	std::vector<std::pair<float, SVec3>> sKeys;
	std::vector<std::pair<float, SQuat>> rKeys;
	std::vector<std::pair<float, SVec3>> pKeys;
};


class Animation
{
private:

	std::string _name;

	std::map<std::string, AnimChannel> _channels;

	double _ticks, _ticksPerSecond, _duration, _tickDuration, _invTickDuration;
	

public:

	Animation();

	Animation(std::string& name, double ticks, double ticksPerSecond, int nc) : _name(name), _ticks(ticks), _ticksPerSecond(ticksPerSecond)
	{
		_duration = _ticks / _ticksPerSecond;
		_tickDuration = _duration / _ticks;
		_invTickDuration = 1.0 / _duration;
		//_channels.reserve(nc);
	}



	~Animation();


	void addChannel(const AnimChannel& ac)
	{
		_channels.insert({ ac.jointName, ac });
	}



	inline float getTickDuration()	const { return _tickDuration; }



	inline float getDuration()		const { return _duration; }



	bool getAnimChannel(std::string name, AnimChannel& ac) const 
	{ 
		auto it = _channels.find(name);
		
		if (it != _channels.end())
		{
			ac = it->second;
			return true;
		}
		return false;
	}


	std::string getName() { return _name; }



	void getTransformAtTime(Bone& joint, std::vector<SMatrix>& vec, const SMatrix& parentMatrix, const SMatrix& glInvT, float elapsed) const
	{
		float currentTick = elapsed / _tickDuration;
		float t = fmod(elapsed, _tickDuration);

		AnimChannel channel;
		bool found = getAnimChannel(joint.name, channel);

		// This matrix is created according to animation data and current time using interpolation, if no data use default
		
		SMatrix animTransform;

		if (found)
		{
			animTransform = getInterpolatedTransform(channel, currentTick, t);	// Animation found, use it
		}
		else
		{
			animTransform = joint.localTransform;	// No animation channels for this bone right now, use default
		}


		SMatrix nodeTransform = animTransform;
		//nodeTransform *= joint.globalTransform;
		nodeTransform *= parentMatrix;

		SVec4 pretendHipVert(0., 120., 0., 1.);

		SVec4 lbs = SVec4::Transform(pretendHipVert, joint.meshToBoneTransform);
		SVec4 abs = SVec4::Transform(lbs, nodeTransform);
		SVec4 fms = SVec4::Transform(abs, glInvT);

		SMatrix finalMatrix = joint.meshToBoneTransform;	// Go from mesh space to bone space
		finalMatrix *= nodeTransform;						// Animate					
		finalMatrix *= glInvT;								// Do uh... something, this is idt anyways

		SVec4 control = SVec4::Transform(pretendHipVert, finalMatrix);

		// @TODO move responsibility for this to calling code
		vec[joint.index] = finalMatrix.Transpose();	//transpose because the shader is column major, nothing to do with the animation process

		for (Bone* child : joint.offspring)
			getTransformAtTime(*child, vec, nodeTransform, glInvT, elapsed);
	}



	SMatrix getInterpolatedTransform(const AnimChannel& channel, float currentTick, float t) const
	{
		SVec3 pos, scale;
		SQuat quat;

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

		return (sMat * rMat * tMat);	// V * S * R * T
	}
};