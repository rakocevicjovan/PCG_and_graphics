#pragma once
#include "Math.h"
#include <vector>

// should be a convex shape... how to do this though, is a big ??? for now... got to read up on it
struct NavBlock
{
	std::vector<SVec3> points;
};



struct NavEdge
{
	//two fields that share this border
	NavBlock* left;
	NavBlock* right;
	int li1, li2, ri1, ri2;	//indices in the points vector in both nodes
};



class NavMesh
{


};

