#pragma once
#include "Math.h"
#include "Mesh.h"
#include <vector>
#include <string>

#include <fbxsdk.h>



class FBXLoader
{
private:

	FbxManager* _fbxMan;		//std::unique_ptr<FbxManager> _fbxMan; std::make_unique(FbxManager);
	FbxIOSettings* _fbxIOs;

	char* cad;

public:

	void init()
	{
		_fbxMan = FbxManager::Create();	
		_fbxIOs = FbxIOSettings::Create(_fbxMan, IOSROOT);
		_fbxMan->SetIOSettings(_fbxIOs);
		cad = new char[4096];
	}



	void parse(const std::string& path)
	{
		FbxImporter* importer = FbxImporter::Create(_fbxMan, "");

		if (!importer->Initialize(path.c_str(), -1, _fbxMan->GetIOSettings()))
		{
			OutputDebugStringA("Call to FbxImporter::Initialize() failed.\n");

			std::string errStr = "Error returned: \n" + std::string(importer->GetStatus().GetErrorString());
			OutputDebugStringA(errStr.c_str());
			exit(-1);
		}

		FbxScene* scene = FbxScene::Create(_fbxMan, "myScene");
		importer->Import(scene);
		importer->Destroy();

		FbxNode* lRootNode = scene->GetRootNode();
		if (lRootNode)
		{
			for (int i = 0; i < lRootNode->GetChildCount(); i++)
				PrintNode(lRootNode->GetChild(i));
		}
		
		// Destroy the SDK manager and all the other objects it was handling.
		cleanUp();
	}



private:

	void parseNode(FbxNode* pNode)
	{

	}


	void cleanUp()
	{
		_fbxMan->Destroy();	//delete _fbxMan; I will assume Destroy() takes care of everything, dtor is inaccessible
	}






	// Printing code, pretty useless for anything but tedious debugging but meh

	void PrintNode(FbxNode* pNode)
	{
		static UINT numTabs = 0;

		PrintTabs(numTabs);
		const char* nodeName = pNode->GetName();
		FbxDouble3 translation = pNode->LclTranslation.Get();
		FbxDouble3 rotation = pNode->LclRotation.Get();
		FbxDouble3 scaling = pNode->LclScaling.Get();
		
		sprintf(cad, "<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
			nodeName,
			translation[0], translation[1], translation[2],
			rotation[0], rotation[1], rotation[2],
			scaling[0], scaling[1], scaling[2]);  
		OutputDebugStringA(cad);

		++numTabs;

		// Print the node's attributes.
		for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
			PrintAttribute(pNode->GetNodeAttributeByIndex(i), numTabs);

		// Recursively print the children.
		for (int j = 0; j < pNode->GetChildCount(); j++)
			PrintNode(pNode->GetChild(j));

		--numTabs;
		PrintTabs(numTabs);
		//printf("</node>\n");
		OutputDebugStringA("</node>\n");
	}



	void PrintTabs(UINT numTabs)
	{
		for (int i = 0; i < numTabs; i++)
			OutputDebugStringA("\t");
	}



	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type)
	{
		switch (type)
		{
		case FbxNodeAttribute::eUnknown: return "unidentified";
		case FbxNodeAttribute::eNull: return "null";
		case FbxNodeAttribute::eMarker: return "marker";
		case FbxNodeAttribute::eSkeleton: return "skeleton";
		case FbxNodeAttribute::eMesh: return "mesh";
		case FbxNodeAttribute::eNurbs: return "nurbs";
		case FbxNodeAttribute::ePatch: return "patch";
		case FbxNodeAttribute::eCamera: return "camera";
		case FbxNodeAttribute::eCameraStereo: return "stereo";
		case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
		case FbxNodeAttribute::eLight: return "light";
		case FbxNodeAttribute::eOpticalReference: return "optical reference";
		case FbxNodeAttribute::eOpticalMarker: return "marker";
		case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
		case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
		case FbxNodeAttribute::eBoundary: return "boundary";
		case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
		case FbxNodeAttribute::eShape: return "shape";
		case FbxNodeAttribute::eLODGroup: return "lodgroup";
		case FbxNodeAttribute::eSubDiv: return "subdiv";
		default: return "unknown";
		}
	}



	void PrintAttribute(FbxNodeAttribute* pAttribute, UINT numTabs)
	{
		if (!pAttribute)
			return;

		FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
		FbxString attrName = pAttribute->GetName();
		PrintTabs(numTabs);

		// Note: to retrieve the character array of a FbxString, use its Buffer() method.
		sprintf(cad, "<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
		OutputDebugStringA(cad);
	}
};