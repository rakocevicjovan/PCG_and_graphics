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
};



class Hexer
{
	ID3D11Device* _device;
	CollisionEngine* colEng;
	float _lifeSpan = 25.f;
	float cellSize = 10.f;
	std::vector<SVec3> dirs;
	Model model;

public:
	Hexer() {};
	~Hexer() {}

	Platform root;
	std::vector<Platform> platforms;

	void init();
	void addPlatform(Platform p, int direction);
	void update(float dTime);
};