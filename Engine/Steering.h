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
	static SVec3 separate(NavAgent& me, const std::vector<NavAgent>& theBois)
	{
		SVec3 result = SVec3::Zero;
		int counter = 0;			//remove this idiocy when octree neighbours within radius is figured out

		if (theBois.empty())
			return result;

		for (const NavAgent& boi : theBois)
		{
			SVec3 separator = -static_cast<SVec3>(boi.getPosition() - me.getPosition());
			float distance = max(0.0001f, separator.Length());

			//temporary fix for only checking near ones... octree should do this @TODO
			if (distance < 3.f)
			{
			separator /= distance;
			float intensityAdjustment = distance;//Math::smoothstep(2.f, 0.f, distance);
			separator *= intensityAdjustment;
			result += separator;	//me->r
			counter++;
			}
		}

		result /= counter;
		return result;		// * agent.maxForce... not sure wth that is though, and I need another class not
	}



	template <typename NavAgent>
	static SVec3 stick(NavAgent* me, std::vector<NavAgent*>& theBois)
	{
		if (theBois.empty())
			return SVec3::Zero;

		SVec3 centerOfMass = me->getPosition();

		for (NavAgent* boi : theBois)
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