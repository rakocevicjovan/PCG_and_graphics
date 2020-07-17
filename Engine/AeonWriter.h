#pragma once
#include "FileUtilities.h"
#include <iostream>
#include <memory>



class AeonWriter
{
private:

	bool _active;

public:

	std::string _exportPath;



	AeonWriter() : _active(false), _exportPath("C:\\Users\\Senpai\\Desktop\\AeonTest") {}



	bool displayExportSettings()
	{
		bool result = false;

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
					result = true;
			}

			if(!result)
				result = displayOverwriteWarning();
		}
		ImGui::End();

		return result;
	}



	bool displayOverwriteWarning()
	{
		bool result = false;

		if (ImGui::BeginPopup("File already exists!"))
		{
			ImGui::Text("Are you sure you want to overwrite it?");

			if (ImGui::Button("Overwrite"))
			{
				ImGui::CloseCurrentPopup();
				result = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("No"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
		return result;
	}



	inline void activate() { _active = true; }

	inline void deactivate() { _active = false; }

	inline bool isActive() { return _active; }
};