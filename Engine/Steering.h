#pragma once
#include "Math.h"
#include <vector>
#include "Hull.h"

class Steering
{
public:

	//trivial but I don't even need it rn...
	static SVec3 seek(SVec3 pos)
	{

	}


	template <typename NavAgent>
	static SVec3 separate(NavAgent* me, std::vector<NavAgent*>& theBois)
	{
		if (theBois.empty())
			return SVec3::Zero;

		SVec3 result = SVec3::Zero;

		for (SphereHull* boi : theBois)
		{
			result += SVec3(1) - static_cast<SVec3>((me->getPosition() - boi->getPosition()) / me->r);
		}

		result /= theBois.size();
		return result;		// * agent.maxForce... not sure wth that is though, and I need another class not
	}


	template <typename NavAgent>
	static SVec3 stick(NavAgent* me, std::vector<NavAgent*>& theBois)
	{
		if (theBois.empty())
			return SVec3::Zero;

		SVec3 centerOfMass = me->getPosition();

		for (SphereHull* boi : theBois)
		{
			centerOfMass += boi->getPosition();
		}

		centerOfMass /= (theBois.size() + 1);

		SVec3 direction = centerOfMass - me->getPosition();
		//direction *= me->maxSpeed / direction.length();	//normalized and scaled to max speed

		SVec3 force = direction;
		//SVec3 force = direction - me.velocity;
		//force *= (me.maxForce / me.maxSpeed);
		return force;
	}


	template <typename NavAgent>
	static SVec3 alignFacing()
	{

	}


	template <typename NavAgent>
	static SVec3 flock()
	{
		//stick, separate and align
	}
};