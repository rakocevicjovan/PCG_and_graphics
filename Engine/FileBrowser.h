#pragma once
#include "GuiBlocks.h"
#include "FileUtilities.h"

class FileBrowser
{
private:

	uint8_t MAX_UNDOS = 8u;

	std::string _searchedString;

	std::filesystem::path _curDirPath;

	std::deque<std::filesystem::path> _pathHistory;

	std::filesystem::path _selectedItemPath;

	bool _badPath;

public:

	FileBrowser() {}



	FileBrowser(const std::string& rootFolder)
	{
		_curDirPath = std::filesystem::path(rootFolder);

		openDir(_curDirPath);

		_searchedString = _curDirPath.string();

		_pathHistory.push_front(_curDirPath);

		_badPath = false;
	}



	void display()
	{
		if (ImGui::Begin("Asset list"))
		{
			bool tempBadPath = _badPath;	//Since it can change with seek

			if(tempBadPath) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1., 0., 0., 1.));
			
			if (inTextStdStringHint("Path", "Search for assets", _searchedString))
				seek();

			if (tempBadPath) ImGui::PopStyleColor();


			if (ImGui::Button("Seek"))
				seek();

			ImGui::SameLine();

			if (ImGui::Button("Up"))
				stepUp();

			ImGui::SameLine();

			if (ImGui::Button("Back"))
				stepBack();


			ImGui::ListBoxHeader("Contents");

			auto file = std::filesystem::directory_entry(_curDirPath);

			if (file.exists() && file.is_directory())
			{
				std::filesystem::directory_iterator dirIter(_curDirPath);

				for (const std::filesystem::directory_entry& de : dirIter)	//for (int i = 0; i < _contents.size(); i++)
				{
					if (ImGui::Button(de.path().filename().string().data()))	//ImGui::Button(_contents[i].c_str())
					{
						if (de.is_directory())
						{
							_searchedString = de.path().string();
							seek();
						}
						else
						{
							_selectedItemPath = de;
						}
					}
				}
			}

			ImGui::ListBoxFooter();

		}
		ImGui::End();
	}



	void seek()
	{
		_pathHistory.push_back(_curDirPath);
		openDir(std::filesystem::path(_searchedString));
	}



	void stepUp()
	{
		_pathHistory.push_back(_curDirPath);
		openDir(_curDirPath.parent_path());
	}



	bool stepBack()
	{
		if (_pathHistory.empty())
			return false;

		auto mostRecent = _pathHistory.back();
		_pathHistory.pop_back();
		openDir(mostRecent);

		return true;
	}



	void openDir(const std::filesystem::path& newPath)
	{
		if (_pathHistory.size() >= MAX_UNDOS)
			_pathHistory.pop_front();

		if (std::filesystem::is_directory(newPath))
		{
			_curDirPath = newPath;

			_searchedString = _curDirPath.string();

			_badPath = false;
		}
		else
		{
			_badPath = true;
		}
	}
};