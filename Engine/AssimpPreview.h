#pragma once
#include "Model.h"
#include "FileUtilities.h"
#include "GuiBlocks.h"

class AssimpPreview
{
private:

	Assimp::Importer _importer;

	std::string _path;
	const aiScene* _scene;

public:

	bool loadAiScene(const std::string& path, UINT inFlags)
	{
		_path = path;

		assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

		unsigned int pFlags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_PreTransformVertices |
			aiProcess_ConvertToLeftHanded;

		Assimp::Importer importer;
		_scene = importer.ReadFile(path, pFlags);

		if (!_scene || _scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode)
		{
			std::string errString("Assimp error:" + std::string(importer.GetErrorString()));
			OutputDebugStringA(errString.c_str());
			return false;
		}

		//_meshes.reserve(scene->mNumMeshes);
		//processNode(device, scene->mRootNode, scene, scene->mRootNode->mTransformation, rUVx, rUVy);

		return true;
	}
	


	void displayAiScene()
	{

	}

};