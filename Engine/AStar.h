#pragma once
#include "NavMesh.h"

//#undef max	//fuck you minwindef.h seriously...
//this is possibly dangerous if used elsewhere and  the push macro thing looks horrible so I'm using parentheses instead...

template <float (*calcHeuristic)(const NavNode& a, const NavNode& b)>
class AStar
{
private:
	template <typename NavNodeType>
	static void prepareGraph(std::vector<NavNodeType>& nodes, int goalIndex)
	{
		for (auto& node : nodes)
		{
			node.pathWeight = (std::numeric_limits<float>::max)();
			node.visited = false;
			node.pathPredecessor = -1;
		}
		nodes[goalIndex].pathWeight = 0;
	}


	template <typename NavEdgeType>
	static int getNeighbourIndex(const NavEdgeType& edge, int myIndex)
	{
		return edge.first == myIndex ? edge.last : edge.first;
	}


	template <typename NavNodeType, typename NavEdgeType>
	static void visitNeighbours(std::vector<NavNodeType>& nodes, const std::vector<NavEdgeType>& edges, int curNodeIndex, int goalIndex)
	{
		NavNodeType& curNode = nodes[curNodeIndex];

		for (int edgeIndex : curNode.edges)	//check all neighbours
		{
			const NavEdgeType& curEdge = edges[edgeIndex];							//by iterating through edges
			NavNodeType& curNbr = nodes[getNeighbourIndex(curEdge, curNodeIndex)];	//and getting the node that isn't curNode

			if (curNbr.visited)	//ignore the previously visited nodes!
				continue;

			float curNbrWeight = curNbr.pathWeight;		//get the current cost of pathing through the neighbours

			float gWeight = curNode.pathWeight + curEdge.weight;	//get the new cost based on g
			float hWeight = calculateHeuristic(curNbr, nodes[goalIndex]);

			float newPathCost = gWeight + hWeight;

			if (newPathCost < curNbrWeight)	//if the path from cur node to neighbour is "cheaper" then nbr's original path
			{
				curNbr.pathPredecessor = curNodeIndex;	//readjust the path to the new, cheaper version
				curNbr.pathWeight = newPathCost;
			}
		}
	}


	template <typename NavNodeType, typename NavEdgeType>
	inline static void visitNode(std::vector<NavNodeType>& nodes, std::vector<NavEdgeType>& edges, int navNodeIndex, int goalIndex)
	{
		visitNeighbours(nodes, edges, navNodeIndex, goalIndex);
		nodes[navNodeIndex].visited = true;
	}



	inline static float calculateHeuristic(const NavNode& a, const NavNode& b)
	{ 
		return calcHeuristic(a, b);
	}

public:

	template <typename NavNodeType, typename NavEdgeType>
	static void fillGraph(std::vector<NavNodeType>& nodes, std::vector<NavEdgeType>& edges, int goalIndex)
	{
		prepareGraph(nodes, goalIndex);

		int curNodeIndex = goalIndex;

		while (curNodeIndex > -1)
		{
			visitNode(nodes, edges, curNodeIndex, goalIndex);

			float minPathWeight = (std::numeric_limits<float>::max)();
			curNodeIndex = -1;

			//alternative is using std::make_heap for unvisited nodes, and popping all the time
			//I suspect it might faster this way as iteration is fast and cache friendly especially with modern optimized loops
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
			//Instead, I simply continue until the reachable nodes are checked because I'm filling the flow field
		}
	}
};