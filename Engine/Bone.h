#pragma once
#include "Math.h"
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>

class Bone
{
public:

	int _index;
	std::string _name;

	SMatrix _offsetMatrix;
	SMatrix _localMatrix;
	//SMatrix _globalMatrix; was useful for debugging, not any more really

	Bone* _parent = nullptr;
	std::vector<Bone*> _children;

	Bone() : _parent(nullptr) {}

	Bone(int index, std::string name, SMatrix offset)
		: _index(index), _name(name), _offsetMatrix(offset), _parent(nullptr) {}

	// Again, can't serialize the tree because of pointers...
	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_index, _name, _offsetMatrix, _localMatrix);
	}
};