#include "Geometry.h"
#include "Math.h"
#include "Model.h"
#include "CollisionEngine.h"

struct Platform
{
	Platform() {}
	Platform(SVec3 position) : position(position) {}

	SVec3 position;
	float age = 0.f;
	bool active = true;
};



struct AxialCoords
{
	AxialCoords() {}
	AxialCoords(int q, int r) : q(q), r(r) {}

	int q, r;
};



struct HexGridCell
{
	CellKey ck;


};



class Hexer
{
	ID3D11Device* _device;
	CollisionEngine* colEng;
	float _lifeSpan = 150.f, _cellSize = 0.f, _cellDist = 0.f, _triHeight = 0.f;
	std::vector<SVec3> dirs;
	Model model;

	std::vector<CellKey> cubeDirs =
	{ 
		CellKey(+1, -1, 0), CellKey(+1, 0, -1), CellKey(0, +1, -1), CellKey(-1, +1, 0), CellKey(-1, 0, +1), CellKey(0, -1, +1)
	};

public:
	Hexer() {};
	~Hexer() {}

	Platform _root;
	SVec3 _lastPlatformPos;
	std::vector<Platform> _platforms;
	std::vector<SVec3> _obstacles;

	void init(float cellSize, SVec3 root);
	void addPlatform(SVec3 parentPos, int direction);
	void update(float dTime);
	void createObstacleCourse();

	SVec3 getCornerPos(const SVec3& center, UINT i);
	CellKey axialToCube(AxialCoords ac);
	AxialCoords cubeToAxial(CellKey ck);
};