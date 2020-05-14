#pragma once
#include "Steering.h"
#include "NavGrid.h"
#include "Math.h"

template <typename ParentType>
class SteeringComponent
{
public:
	SVec3 _totalInfluence = SVec3::Zero;
	float _mspeed = 100.f;
	float _radius = 1.f;
	ParentType* _parent;

	//notes whether there an assigned movement intent exists
	//this needs to be set to true from a higher place in gameplay code that issues movement commands
	//to avoid useless steering calculations, we set it to false upon arrival
	bool _active = true;	



	SteeringComponent(ParentType* parent)
	{
		_parent = parent;
	}

	//I could also make this T but that would make it necessary to compare equal types...
	//best solution is other steering components I guess?
	template <typename NavAgent>
	void update(const NavGrid& navGrid, float dTime, const std::vector<NavAgent*>& others, int index, float stopDistance)
	{
		if (!_active) return;

		SVec3 myPos = _parent->getPosition();
		int creepsCell = navGrid.posToCellIndex(myPos);
		SVec3 flowVector = navGrid.flowAtIndex(creepsCell);
		
		SVec3 obstacleCorrection = navGrid.flowObstacleCorrection(myPos);
		flowVector += 1.f * obstacleCorrection;

		SVec3 goalPos = navGrid.cellIndexToPos(navGrid.getGoalIndex());
		SVec3 vecToGoal = myPos - goalPos;
		float sqDistToGoal = vecToGoal.LengthSquared();

		//seeking using the flowfield
		if (sqDistToGoal > stopDistance * stopDistance)	//sqDistToGoal > stopDistance * stopDistance
		{
			_totalInfluence += flowVector;

			SVec3 separation = .2f * Steering::separate(static_cast<NavAgent*>(_parent), others);
			_totalInfluence += separation;
		}
		else	//arrival behaviour
		{
			SVec3 desiredPos = goalPos + 4.f * SVec3(index % 10, 0, (index / 10) % 10);
			_totalInfluence += Math::getNormalizedVec3(desiredPos - myPos);

			//a bit hacky really... but it allows collision to overtake and sort them out instead of fighting it
			if ((SVec2(desiredPos.x, desiredPos.z) - SVec2(myPos.x, myPos.z)).LengthSquared() < (2.f * _radius) * (2.f * _radius))
				_active = false;
		}

		if (_totalInfluence.LengthSquared() < 0.0001)
			return;

		_totalInfluence.Normalize();

		Math::Translate(_parent->_transform, _totalInfluence * _mspeed * dTime);
		_totalInfluence = SVec3::Zero;
	}
};