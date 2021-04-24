#include "pch.h"
#include "AnimChannel.h"


AnimChannel::AnimChannel(int p, int r, int s) noexcept
{
	_pKeys.reserve(p);
	_rKeys.reserve(r);
	_sKeys.reserve(s);
}



SMatrix AnimChannel::getInterpolatedTransform(float currentTick, float t) const
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
		auto it = std::upper_bound(_pKeys.begin(), _pKeys.end(), currentTick,
			[](float currentTick, PosFrame frame) -> bool
			{
				return currentTick < frame.tick;
			});

		pos = Math::lerp((it - 1)->pos, it->pos, t);
	}

	UINT numSCh = _sKeys.size();
	if (numSCh == 1u)
	{
		scale = _sKeys[0].scale;
	}
	else
	{
		auto it = std::upper_bound(_sKeys.begin(), _sKeys.end(), currentTick,
			[](float currentTick, SclFrame frame) -> bool
			{
				return currentTick < frame.tick;
			});

		scale = Math::lerp((it - 1)->scale, it->scale, t);
	}


	UINT numRCh = _rKeys.size();
	if (numRCh == 1u)
	{
		quat = _rKeys[0].rot;
	}
	else
	{
		auto it = std::upper_bound(_rKeys.begin(), _rKeys.end(), currentTick,
			[](float rhs, RotFrame lhs) -> bool {return rhs < lhs.tick; });

		SQuat rotPost = it->rot;
		--it;
		SQuat rotPre = it->rot;
		quat = SQuat::Slerp(rotPre, rotPost, t);
	}

	return DirectX::XMMatrixAffineTransformation(scale, SVec3(0.f), quat, pos);
}



/* Old way to look for the frame, might be good to keep around... New doesn't justWorkTM on reversed animations 
 * but it could be made to, which I'd prefer to this naive linear search

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

	for (UINT i = 0; i < numRCh; ++i)
	{
		int nextTick = i + 1 == numRCh ? 0 : i + 1;

		if (currentTick < (float)_rKeys[nextTick].tick)
		{
			SQuat rotPre = _rKeys[i].rot;
			SQuat rotPost = _rKeys[nextTick].rot;
			quat = Math::lerp(rotPre, rotPost, t);
			break;
		}
	}
		
	for (UINT i = 0; i < numSCh; ++i)
	{
		int nextTick = ((i + 1) == numSCh) ? 0 : i + 1;

		if (currentTick < (float)_sKeys[nextTick].tick)
		{
			SVec3 scalePre = _sKeys[i].scale;
			SVec3 scalePost = _sKeys[nextTick].scale;
			scale = Math::lerp(scalePre, scalePost, t);
			break;
		}
	}
*/