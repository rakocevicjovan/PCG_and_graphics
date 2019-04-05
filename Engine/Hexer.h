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



struct CubeCoords
{
	CubeCoords() {};
	CubeCoords(int x, int y, int z) : x(x), y(y), z(z) {}

	static CubeCoords cubeAdd(CubeCoords c1, CubeCoords c2) { return CubeCoords(c1.x + c2.x, c1.y + c2.y, c1.z + c2.z); }
	static std::vector<CubeCoords> cubeDirs;


	int x, y, z;
};



struct AxialCoords
{
	AxialCoords() {}
	AxialCoords(int q, int r) : q(q), r(r) {}

	static std::vector<AxialCoords> axialCoords;

	int q, r;
};



struct HexGridCell
{
	CubeCoords ck;

};



class Hexer
{
	ID3D11Device* _device;
	CollisionEngine* colEng;
	float _lifeSpan = 150.f, _cellSize = 0.f, _cellDist = 0.f, _triHeight = 0.f;
	std::vector<SVec3> dirs;
	Model model;

public:
	Hexer() {};
	~Hexer() {}

	Platform _root;
	SVec3 _lastPlatformPos;
	std::vector<Platform> _platforms;
	std::vector<SVec3> _points = 
	{
		SVec3(190, 128, 185), SVec3(508, 128, 208),
		SVec3(417, 128, 570), SVec3(243, 128, 782),
		SVec3(675, 128, 896), SVec3(768, 128, 782)
	};

	void init(float cellSize, SVec3 root);
	void addPlatform(SVec3 parentPos, int direction);
	void update(float dTime);
	void createObstacleCourse();

	SVec3 getCornerPos(const SVec3& center, UINT i);
	CubeCoords axialToCube(AxialCoords ac);
	AxialCoords cubeToAxial(CubeCoords ck);
	CubeCoords getCubeDir(UINT direction);
	CubeCoords getNeighbourAtDirection(CubeCoords cube, UINT direction);
};