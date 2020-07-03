#pragma once
#include "AssimpPreview.h"
#include "FileUtilities.h"
#include "Serializer.h"
#include <iostream>
#include <memory>



class AeonWriter
{
private:

	bool _active;

	void* outputItem;	// Boo hoo voidptr evil

	bool doMod;
	bool doSkel;
	bool doTex;
	bool doAnim;

public:

	std::string _exportPath;



	AeonWriter() :
		_active(false), doMod(true), doSkel(true), doTex(true), doAnim(true),
		_exportPath("C:\\Users\\Senpai\\Desktop\\Test.txt")
	{}



	bool displayExportSettings()
	{
		bool result = false;

		if (ImGui::Begin("Export panel"))
		{
			// Settings
			inTextStdString("Export path", _exportPath);

			ImGui::Checkbox("Model", &doMod);
			ImGui::Checkbox("Skeleton", &doSkel);
			ImGui::Checkbox("Textures", &doTex);
			ImGui::Checkbox("Animations", &doAnim);

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

			if (ImGui::Button("JUST DO IT!"))
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



	void exportAsset(MemChunk& mc)
	{
		FileUtils::writeAllBytes(_exportPath.c_str(), mc.ptr.get(), mc.size);
		deactivate();
	}



	inline void activate() { _active = true; }

	inline void deactivate() { _active = false; }

	inline bool isActive() { return _active; }
};