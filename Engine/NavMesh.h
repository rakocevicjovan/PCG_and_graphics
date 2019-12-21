#pragma once
#include "Math.h"
#include "NavGraphTypes.h"



// should be a convex shape... how to enforce this though, is a big ??? for now... got to read up on it
struct NavMeshNode : public NavNode
{
	std::vector<SVec3> points;
};



//could use recast... really don't know how to do this myself under a month of work...
class NavMesh
{


};