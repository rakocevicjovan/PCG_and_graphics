#pragma once
#include "Math.h"
#include "Model.h"
#include "Chaos.h"

namespace Procedural
{

struct MazeCell
{
	MazeCell() { }
	MazeCell(int set, int x, int z) : set(set), x(x), z(z) { }
	MazeCell(int x, int z) : x(x), z(z) { }

	int set{ -1 };
	int x{ 0 };
	int z{ 0 };

	bool t{ true };
	bool r{ true };
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
	uint32_t _w{ 0u };
	uint32_t _h{ 0u };
	float _cellSize{0.f};
	float _height{ 30.f };
	float _thickness{ 10.f };

public:

	Model model;

	void Init(uint32_t w, uint32_t h, float cellSize);
	
	void Eller();
	void PopulateRow(int z, std::map<int, EllerSet>& currentRow);
	void CreateModel(ID3D11Device* pDevice);
	void BuildCellMeshes(MazeCell& mc, ID3D11Device* pDevice, Mesh& left, Mesh& right, Mesh& top, Mesh& bottom);
	void AlignWall(MazeCell& mc, const Mesh& m, ID3D11Device* pDevice);

	SVec3 GetRandCellPos();
};

}
