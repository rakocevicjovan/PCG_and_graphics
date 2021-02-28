#pragma once
#include "Math.h"
#include <vector>

class LinearSceneGraph
{
	/*
public:

	// Linear data structure, keep sorted according to parent
	struct SGNode
	{
		uint32_t _parent;
		SMatrix _localTransform;
	};

private: 

	std::vector<SGNode> _nodes;

public:
*/

public:

	struct Link
	{
		int pID;
		int cID;
	};

	std::vector<Link> links;

	// Breaks if either moves!
	//void link(SMatrix& p, SMatrix& c){}

	void link(int pID, int cID)
	{
		links.push_back({ pID, cID });
	}

	void unlink(int linkIndex)
	{
		links[linkIndex] = std::move(links.back());
		links.pop_back();
	}

	void update(std::vector<SMatrix>& transforms)
	{
		for (auto& link : links)
		{
			transforms[link.cID] *= transforms[link.pID];
		}
	}

};