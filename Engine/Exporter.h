#pragma once
#include "AssimpPreview.h"
#include "FileUtilities.h"



class Exporter
{
private:

	bool _active;

public:

	std::string _exportPath;

	Exporter() : _active(false), _exportPath("C:\\Users\\Senpai\\Desktop\\Test.txt")
	{}


	void displayExportSettings()
	{

		// Move this to a separate class, it could get big...
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
		char* test = "TEST";
		FileUtils::writeAllBytes(_exportPath.c_str(), test, 4);
	}



	// Export functions for asset classes
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
		memcpy(&output, &w, metadataSize);
		memcpy(&output + 16, data, dataSize);

		FileUtils::writeAllBytes(exportPath.c_str(), &output, metadataSize + dataSize);

		// Too naive, needs to be replicated otherwise
		//FileUtils::writeAllBytes(exportPath.c_str(), &texture, sizeof(Texture));
	}

	inline void activate() { _active = true; }
	inline void deactivate() { _active = false; }
	inline bool isActive() { return _active; }
};