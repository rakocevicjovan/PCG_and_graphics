#pragma once
#include "Math.h"
#include "Model.h"
#include "Chaos.h"
#include <vector>
#include <map>


namespace Procedural
{

	struct MazeCell
	{
		MazeCell() { }
		MazeCell(int set, int x, int z) : set(set), x(x), z(z) { }
		MazeCell(int x, int z) : x(x), z(z) { }

		int set = -1;
		int x = 0, z = 0; 

		bool t = true;
		bool r = true;
	};



	struct EllerSet
	{
		std::vector<int> cellIDs;

		EllerSet() {}
		EllerSet(int cellID) { cellIDs.push_back(cellID); }
	};



class Maze
{
	std::vector<MazeCell> cells;
	unsigned int _w, _h;
	float _cellSize;
	float _height = 30.f;
	float _thickness = 10.f;

public:

	Model model;

	Maze();
	~Maze();

	void Init(unsigned int w, unsigned int h, float cellSize);
	
	void Eller();
	void PopulateRow(int z, std::map<int, EllerSet>& currentRow);
	void CreateModel(ID3D11Device* pDevice);
	void BuildCellMeshes(MazeCell& mc, ID3D11Device* pDevice, Mesh& left, Mesh& right, Mesh& top, Mesh& bottom);
	void AlignWall(MazeCell& mc, const Mesh& m, ID3D11Device* pDevice);

	SVec3 GetRandCellPos();
};

}