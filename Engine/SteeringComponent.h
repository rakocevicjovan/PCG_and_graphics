#pragma once
#include "Steering.h"
#include "NavGrid.h"
#include "Math.h"

template <typename T>
class SteeringComponent
{
public:
	SVec3 _cumulativeMovement = SVec3::Zero;
	float _mspeed = 100.f;
	float _radius = 1.f;
	T* _parent;

	//notes whether there an assigned movement intent exists
	//this needs to be set to true from a higher place in gameplay code that issues movement commands
	//to avoid useless steering calculations, we set it to false upon arrival
	bool _active = true;	



	SteeringComponent(T* parent)
	{
		_parent = parent;
	}

	//I could also make this T but that would make it necessary to compare equal types...
	//best solution is other steering components I guess?
	template <typename NavAgent>
	void update(const NavGrid& navGrid, float dTime, const std::list<NavAgent*>& others, int index, float stopDistance)
	{
		if (!_active)
			return;

		SVec3 myPos = _parent->getPosition();
		int creepsCell = navGrid.posToCellIndex(myPos);
		SVec3 flowVector = navGrid.flowAtIndex(creepsCell);
		SVec3 goalPos = navGrid.cellIndexToPos(navGrid.getGoalIndex());
		SVec3 vecToGoal = myPos - goalPos;
		float distToGoal = vecToGoal.Length();

		//seeking using the flowfield
		if (distToGoal > stopDistance)
		{
			_cumulativeMovement += flowVector;
			_cumulativeMovement += Steering::separate(static_cast<T>(*_parent), others);
		}
		else	//arrival behaviour
		{
			SVec3 desiredPos = goalPos + 2.f * SVec3(index % 10, 0, (index / 10) % 10);
			
			//a bit hacky really... but it allows collision to overtake and sort them out instead of fighting it
			//if ((SVec2(desiredPos.x, desiredPos.z) - SVec2(myPos.x, myPos.z)).LengthSquared() < (2.f * _radius) * (2.f * _radius))
				//_active = false;

			_cumulativeMovement += Math::getNormalizedVec3(desiredPos - myPos);
			
		}

		if (_cumulativeMovement.LengthSquared() < 0.0001)
			return;

		_cumulativeMovement.Normalize();

		Math::Translate(_parent->transform, _cumulativeMovement * _mspeed * dTime);
		_cumulativeMovement = SVec3::Zero;
	}
};