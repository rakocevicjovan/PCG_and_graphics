#pragma once
#include <d3d11.h>
#include "Math.h"
#include "Texture.h"
#include <vector>

struct Vert3D 
{
	SVec3 pos;
	SVec2 texCoords;
	SVec3 normal;
	SVec3 tangent;

	Vert3D() {};
	Vert3D(const SVec3& position) : pos(position) {};
};



struct VertBoneData 
{
	unsigned int ids[4];
	float weights[4];
};



//@TODO jointInfluences might need changing... should see how this works first...
struct BonedVert3D : Vert3D 
{
	VertBoneData boneData;
};



class Joint
{
public:

	int index;
	std::string name;
	SMatrix offset;
	std::vector<Joint*> offspring;

	Joint() {}

	Joint(int index, std::string name, SMatrix offset) 
	{
		this->index = index;
		this->name = name;
		this->offset = offset;
	}
};



class Channel
{
public:
	
	std::vector<std::pair<double, Joint>> timeline;

	Channel() {}

	Channel(std::vector<double>& timestamps, std::vector<Joint>& transforms) 
	{
		assert(timestamps.size() == transforms.size());	//@TODO check yourself before you wreck yourself

		for (unsigned int i = 0; i < timestamps.size(); ++i) 	
			timeline.push_back(std::make_pair(timestamps[i], transforms[i]));
	}
};



struct Colour
{
	unsigned char col[4];

	Colour(int r, int g, int b, int a)
	{
		col[0] = (unsigned char)r;
		col[1] = (unsigned char)g;
		col[2] = (unsigned char)b;
		col[3] = (unsigned char)a;
	}
};



struct ColourHDR
{
	float _r, _g, _b, _a;

	ColourHDR(float r, float g, float b, float a)
	{
		_r = r;
		_g = g;
		_b = b;
		_a = a;
	}
};