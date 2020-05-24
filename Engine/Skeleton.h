#pragma once
#include <string>
#include "MeshDataStructs.h"
#include "Math.h"

#include "assimp\Importer.hpp"	
#include "assimp\scene.h"
#include "assimp\postprocess.h" 

// @TODO get rid of the map, it was a learning crutch, too slow - transform to vector of bones with indices as "names"
class Skeleton
{
private:

public:

	std::map<std::string, Bone> _boneMap;
	Bone* _root;
	SMatrix _globalInverseTransform;


	void makeLikeATree(const aiNode* node, SMatrix parentMatrix);



	void linkToParentBone(const aiNode* node, Bone& currentBone);



	void calcGlobalTransforms(Bone& bone, const SMatrix& parentTransform);



	inline bool boneExists(const std::string& name)
	{
		return (_boneMap.find(name) != _boneMap.end());
	}


	// Returns -1 if not found
	inline int getBoneIndex(const std::string& name)
	{
		auto found = _boneMap.find(name);
		return found != _boneMap.end() ? found->second.index : -1;
	}



	bool insertBone(const Bone& bone)
	{
		return (_boneMap.insert({ bone.name, bone }).second);
	}
};