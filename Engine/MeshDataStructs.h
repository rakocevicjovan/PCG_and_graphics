#include "Math.h"

struct Vert3D {

	SVec3 pos;
	//float p1;
	SVec2 texCoords;
	//float p2[2];
	SVec3 normal;
	//float p3;
};

struct BonedVert {
	Vert3D vert;
	float joints[4];
};

class Joint {
public:

	int index;
	std::string name;
	SMatrix offset;

	Joint() {}

	Joint(int index, std::string name, SMatrix offset) {
		this->index = index;
		this->name = name;
		this->offset = offset;
	}
};