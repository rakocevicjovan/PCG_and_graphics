#pragma once
#include "Math.h"
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>


struct PosFrame
{
	SVec3 pos;
	float tick;

	PosFrame() {}
	PosFrame(float tick, SVec3 pos) : pos(pos), tick(tick) {}

	template<class Archive> void serialize(Archive& ar) { ar(pos, tick); }
};


struct RotFrame
{
	SQuat rot;
	float tick;

	RotFrame() {}
	RotFrame(float tick, SQuat rot) : rot(rot), tick(tick) {}

	template<class Archive> void serialize(Archive& ar) { ar(rot, tick); }
};


struct SclFrame
{
	SVec3 scale;
	float tick;

	SclFrame() {}
	SclFrame(float tick, SVec3 scale) : scale(scale), tick(tick) {}

	template<class Archive> void serialize(Archive& ar) { ar(scale, tick); }
};


struct AnimChannel
{
	AnimChannel() {}

	AnimChannel(int p, int r, int s)
	{
		_pKeys.reserve(p);
		_rKeys.reserve(r);
		_sKeys.reserve(s);
	}

	std::string _boneName;

	std::vector<SclFrame> _sKeys;
	std::vector<RotFrame> _rKeys;
	std::vector<PosFrame> _pKeys;

	SMatrix getInterpolatedTransform(float currentTick, float t) const
	{
		SVec3 pos = SVec3(0.);
		SVec3 scale = SVec3(1.);
		SQuat quat = SQuat::Identity;


		UINT numPCh = _pKeys.size();
		if (numPCh == 1u)
		{
			pos = _pKeys[0].pos;
		}
		else
		{
			for (UINT i = 0; i < numPCh; ++i)
			{
				int nextTick = i + 1 == numPCh ? 0 : i + 1;

				if (currentTick < _pKeys[nextTick].tick)
				{
					SVec3 posPre = _pKeys[i].pos;
					SVec3 posPost = _pKeys[nextTick].pos;
					pos = Math::lerp(posPre, posPost, t);
					break;
				}
			}
		}


		UINT numSCh = _sKeys.size();
		if (numSCh == 1u)
		{
			scale = _sKeys[0].scale;
		}
		else
		{
			for (UINT i = 0; i < numSCh; ++i)
			{
				int nextTick = i + 1 == numSCh ? 0 : i + 1;

				if (currentTick < (float)_sKeys[nextTick].tick)
				{
					SVec3 scalePre = _sKeys[i].scale;
					SVec3 scalePost = _sKeys[nextTick].scale;
					scale = Math::lerp(scalePre, scalePost, t);
					break;
				}
			}
		}


		UINT numRCh = _rKeys.size();
		if (numRCh == 1u)
		{
			quat = _rKeys[0].rot;
		}
		else
		{
			for (UINT i = 0; i < numRCh; ++i)
			{
				int nextTick = i + 1 == numRCh ? 0 : i + 1;

				if (currentTick < (float)_rKeys[nextTick].tick)
				{
					SQuat rotPre = _rKeys[i].rot;
					SQuat rotPost = _rKeys[nextTick].rot;
					quat = SQuat::Lerp(rotPre, rotPost, t);	// NLERP is faster than SLERP and still good
					break;
				}
			}
		}

		/* Old way, I'd say a lot slower
		SMatrix sMat = SMatrix::CreateScale(scale);
		SMatrix rMat = SMatrix::CreateFromQuaternion(quat);
		SMatrix tMat = SMatrix::CreateTranslation(pos);

		return (sMat * rMat * tMat);	// V * S * R * T
		*/

		return DirectX::XMMatrixAffineTransformation(scale, SVec3(0.f), quat, pos);
	}


	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(_boneName, _rKeys, _pKeys, _sKeys);
	}
};