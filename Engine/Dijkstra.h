#pragma once
#include "NavMesh.h"
#include <limits>

//#undef max	//fuck you minwindef.h seriously...
//this is possibly dangerous if used elsewhere and  the push macro thing looks horrible so I'm using parentheses instead...

namespace Dijkstra
{
	void prepareGraph(std::vector<NavNode>& nodes, int goalIndex)
	{
		for (auto& node : nodes)
		{
			node.pathWeight = (std::numeric_limits<float>::max)();
			node.visited = false;
			node.pathPredecessor = -1;
		}

		nodes[goalIndex].pathWeight = 0;
	}



	int getNeighbourIndex(const NavEdge& edge, int myIndex)
	{
		return edge.first == myIndex ? edge.last : edge.first;
	}



	void visitNeighbours(std::vector<NavNode>& nodes, const std::vector<NavEdge>& edges, int curNodeIndex)
	{
		NavNode& curNode = nodes[curNodeIndex];

		for (int edgeIndex : curNode.edges)	//check all neighbours
		{
			const NavEdge& curEdge = edges[edgeIndex];							//by iterating through edges
			NavNode& curNbr = nodes[getNeighbourIndex(curEdge, curNodeIndex)];	//and getting the node that isn't curNode

			if (curNbr.visited)	//ignore the previously visited nodes!
				continue;

			float curNbrWeight = curNbr.pathWeight;		//get the current cost of pathing through the neighbours

			float newPathCost = curNode.pathWeight + curEdge.weight;	//get the new cost

			if (newPathCost < curNbrWeight)	//if the path from cur node to neighbour is "cheaper" then nbr's original path
			{
				curNbr.pathPredecessor = curNodeIndex;	//readjust the path to the new, cheaper version
				curNbr.pathWeight = newPathCost;
			}
		}
	}



	void processNode(std::vector<NavNode>& nodes, std::vector<NavEdge>& edges, int navNodeIndex)
	{
		visitNeighbours(nodes, edges, navNodeIndex);
		nodes[navNodeIndex].visited = true;
	}



	void fillGraph(std::vector<NavNode>& nodes, std::vector<NavEdge>& edges, int goalIndex)
	{
		prepareGraph(nodes, goalIndex);

		int curNodeIndex = goalIndex;

		while (curNodeIndex > 0)
		{
			processNode(nodes, edges, curNodeIndex);

			float minPathWeight = (std::numeric_limits<float>::max)();
			curNodeIndex = -1;

			//I suspect it's faster this way then copying between two sets and possibly allocating all the time...
			//iteration is fast and cache friendly
			for (int i = 0; i < nodes.size(); ++i)
			{
				if (nodes[i].visited)
					continue;

				if (nodes[i].pathWeight < minPathWeight)
				{
					minPathWeight = nodes[i].pathWeight;
					curNodeIndex = i;
				}
			}

			//If this was a target search, I'd check if the found node is the target one... 
			//Instead, I simply continue until the reachable nodes are checked
		}
	}
}