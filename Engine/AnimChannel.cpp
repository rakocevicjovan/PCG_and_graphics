#include "AnimChannel.h"
#include <algorithm> 


AnimChannel::AnimChannel(int p, int r, int s)
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
		auto it = std::upper_bound(_rKeys.begin(), _rKeys.end(), currentTick,
			[](float rhs, RotFrame lhs) -> bool {return rhs < lhs.tick; });

		SQuat rotPost = it->rot;
		--it;	// Should be safe as upper_bound will never select the first element, tested it as well
		SQuat rotPre = it->rot;
		quat = SQuat::Slerp(rotPre, rotPost, t);

		/*
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
		}*/
	}

	return DirectX::XMMatrixAffineTransformation(scale, SVec3(0.f), quat, pos);
}


/* Alternative method, not sure if it's faster but I suspect it is
auto it = std::lower_bound(_rKeys.begin(), _rKeys.end(), currentTick,
	[](RotFrame lhs, float rhs) -> bool {return lhs.tick < rhs; });

SQuat rotPre = it++->rot;

if (it == _rKeys.end()) it = _rKeys.begin();	// homebrew ring buffer :V

SQuat rotPost = it->rot;
quat = Math::lerp(rotPre, rotPost, t);
*/

/* Old way to compose matrices, I'd say a lot slower
SMatrix sMat = SMatrix::CreateScale(scale);
SMatrix rMat = SMatrix::CreateFromQuaternion(quat);
SMatrix tMat = SMatrix::CreateTranslation(pos);

return (sMat * rMat * tMat);	// V * S * R * T
*/