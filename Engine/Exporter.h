#pragma once
#include "AssimpPreview.h"
#include "FileUtilities.h"



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
		//exportTexture(outputItem, _exportPath);
	}



	// Export functions for asset classes
	static void exportMesh(const Mesh& mesh, std::string& exportPath)
	{
		UINT indexCount = mesh._indices.size();
		UINT vertexCount = mesh._vertices.size();

		// 48 bytes for 3 vectors, 12 for vbuffer, 8 for IBuffer, 64 for local trf, 36 for material
		const SMatrix* meshLocMat = &mesh._transform;

		char* output;

		// Serializing a material could take some work, leave for later...
		FileUtils::writeAllBytes(exportPath.c_str(), &output, 0);
	}



	static void exportTexture(const Texture& texture, std::string& exportPath)
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