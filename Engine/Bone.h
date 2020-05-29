#pragma once
#include "Math.h"
#include <string>


class Bone
{
public:

	int index;
	std::string name;

	SMatrix _offsetMatrix;
	SMatrix _localMatrix;
	//SMatrix _globalTransform;

	Bone* parent = nullptr;
	std::vector<Bone*> offspring;

	Bone() : parent(nullptr) {}

	Bone(int index, std::string name, SMatrix offset)
		: index(index), name(name), _offsetMatrix(offset), parent(nullptr) {}
};