#pragma once
#include "AssimpPreview.h"
#include "FileUtilities.h"
#include <iostream>



//enum ASS_TYPE { MODEL, SK_MODEL, SKELLY };



class Exporter
{
private:

	bool _active;

	void* outputItem;	// Boo hoo voidptr evil

public:

	std::string _exportPath;



	Exporter() : _active(false), _exportPath("C:\\Users\\Senpai\\Desktop\\Test.txt")
	{}



	void displayExportSettings()
	{
		if (ImGui::Begin("Export panel"))
		{
			// Settings
			inTextStdString("Export path", _exportPath);


			// Controls
			if (ImGui::Button("Close"))
				deactivate();

			ImGui::SameLine();

			if (ImGui::Button("Commit"))
			{
				if (FileUtils::fileExists(_exportPath))
					ImGui::OpenPopup("File already exists!");
				else
					exportAssets();
			}

			displayOverwriteWarning();
		}
		ImGui::End();
	}



	void displayOverwriteWarning()
	{
		if (ImGui::BeginPopup("File already exists!"))
		{
			ImGui::Text("Are you sure you want to overwrite it?");

			if (ImGui::Button("JUST DO IT!"))
			{
				ImGui::CloseCurrentPopup();
				exportAssets();
			}

			ImGui::SameLine();

			if (ImGui::Button("No"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}



	void exportAssets()
	{
	}



	// Export functions for asset classes
	void exportSkeletalModel(const SkeletalModel& skm)
	{
		std::vector<std::pair<char*, UINT>> meshStrings;

		meshStrings.reserve(skm._meshes.size());

		for (int i = 0; i < skm._meshes.size(); ++i)
		{
			const SkeletalMesh& m = skm._meshes[i];

			meshStrings.push_back(exportSkelMesh(m));
		}

		for (int i = 0; i < 1; ++i)	//skm._meshes.size() FOR NOW ONLY ONE
		{
			FileUtils::writeAllBytes(_exportPath.c_str(), meshStrings[i].first, meshStrings[i].second);
		}

		for (auto ms : meshStrings)
		{
			delete ms.first;
		}
	}



	std::pair<char*, UINT> exportSkelMesh(const SkeletalMesh& mesh)
	{
		// Header data
		UINT indexCount = mesh._indices.size();
		UINT vertexCount = mesh._vertices.size();
		//UINT texCount = mesh._textures.size();

		UINT ibs = indexCount * sizeof(UINT);
		UINT vbs = vertexCount * sizeof(BonedVert3D);
		//UINT tbs = texCount * sizeof(UINT);	// These will be indices... not sure how that's gonna work
		
		UINT headerSize = 12;
		UINT dataSize = ibs + vbs;		// + tbs

		UINT totalSize = headerSize + dataSize;

		char* result = new char[totalSize];

		UINT offset = 0u;	// Wrap in a helper if this works

		memcpy(result + offset, &indexCount, sizeof(UINT));
		offset += sizeof(UINT);

		memcpy(result + offset, &vertexCount, sizeof(UINT));
		offset += sizeof(UINT);

		memcpy(result + offset, &vertexCount, sizeof(UINT));
		offset += sizeof(UINT);

		memcpy(result + offset, mesh._indices.data(), ibs);
		offset += ibs;

		memcpy(result + offset, mesh._vertices.data(), vbs);
		offset += vbs;

		return { result, totalSize };
	}



	void exportMesh(const Mesh& mesh, const std::string& exportPath)
	{
		UINT indexCount = mesh._indices.size();
		UINT vertexCount = mesh._vertices.size();

		// 48 bytes for 3 vectors, 12 for vbuffer, 8 for IBuffer, 64 for local trf, 36 for material
		const SMatrix* meshLocMat = &mesh._transform;

		char* output;

		// Serializing a material could take some work, leave for later...
		FileUtils::writeAllBytes(exportPath.c_str(), &output, 0);
	}



	void exportTexture(const Texture& texture, std::string& exportPath)
	{
		// Metadata 4 * 4 bytes, data varies
		int w = texture.getW();
		int h = texture.getH();
		int n = texture.getN();
		TextureRole role = texture._role;

		const unsigned char* data = texture.getData();

		UINT metadataSize = 16;
		UINT dataSize = w * h * n;

		char* output = new char[metadataSize + dataSize];

		// To symmetry... And beyond!
		memcpy(&output[0],	&w,		sizeof(w));
		memcpy(&output[4],	&h,		sizeof(n));
		memcpy(&output[8],	&n,		sizeof(h));
		memcpy(&output[12], &role,	sizeof(role));
		memcpy(&output[16], data,	dataSize);

		FileUtils::writeAllBytes(exportPath.c_str(), &output, metadataSize + dataSize);
	}



	inline void activate() { _active = true; }

	inline void deactivate() { _active = false; }

	inline bool isActive() { return _active; }
};