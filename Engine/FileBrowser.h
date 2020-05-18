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

public:

	FileBrowser() {}



	FileBrowser(const std::string& rootFolder)
	{
		_curDirPath = std::filesystem::path(rootFolder);

		_pathHistory.push_front(_curDirPath);

		openDir(_curDirPath);
	}



	void display()
	{
		if (ImGui::Begin("Asset list"))
		{
			if (inTextStdStringHint("Path", "Search for assets", _searchedString))
				seek();

			if (ImGui::Button("Seek"))
				seek();

			ImGui::SameLine();

			if (ImGui::Button("Up"))
				openDir(_curDirPath.parent_path());

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
							openDir(de.path());
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
		std::filesystem::path newPath(_searchedString);

		if (std::filesystem::is_directory(newPath))
		{
			openDir(newPath);
		}
		else
		{
			// Do what? Popups are clunky and I don't like them!
		}
	}



	void openDir(const std::filesystem::path& newPath)
	{
		if (_pathHistory.size() == MAX_UNDOS)
			_pathHistory.pop_front();

		_pathHistory.push_back(_curDirPath);
		_curDirPath = newPath;

		_searchedString = _curDirPath.string();
	}



	bool stepBack()
	{
		if (_pathHistory.empty())
			return false;

		_curDirPath = _pathHistory.back();
		_pathHistory.pop_back();

		_searchedString = _curDirPath.string();

		return true;
	}
	
};