#pragma once
#include "Math.h"
#include <vector>


#include <string>	//delet later

// should be a convex shape... how to do this though, is a big ??? for now... got to read up on it
struct NavNode
{
	std::string name;
	std::vector<SVec3> points;
	std::vector<int> edges;
	float pathWeight;
	bool visited;
	int pathPredecessor;

	NavNode(std::string n) : name(n) {}
};



struct NavEdge
{
	//int li1, li2, ri1, ri2;	//indices in the points vector in both nodes

	//two fields that share this border
	int first;
	int last;
	float weight;

	NavEdge(int f, int l, int w) : first(f), last(l), weight(w) {}
	NavEdge(int f, int l) : first(f), last(l), weight(1.f) {}
};



class NavMesh
{


};

