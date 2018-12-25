#include "Math.h"
#include <vector>

struct Vert3D {

	SVec3 pos;
	SVec2 texCoords;
	SVec3 normal;
};




//@TODO jointInfluences might need changing... should see how this works first...
struct BonedVert3D {
	Vert3D vert;
	float jointInfluences[4];
};

class Joint {
public:

	int index;
	std::string name;
	SMatrix offset;
	std::vector<Joint*> offspring;

	Joint() {}

	Joint(int index, std::string name, SMatrix offset) {
		this->index = index;
		this->name = name;
		this->offset = offset;
	}
};