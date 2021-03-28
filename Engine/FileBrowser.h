#pragma once
#include "GuiBlocks.h"
#include "FileUtilities.h"
#include <optional>



class FileBrowser
{
private:

	uint8_t _historySize = 8u;

	std::string _searchedString;

	std::filesystem::path _curDirPath;

	std::filesystem::path _projRootPath;	// QoL

	std::deque<std::filesystem::path> _pathHistory;

	std::vector<std::filesystem::directory_entry> _contents;

	bool _badPath;

public:

	FileBrowser() {}



	FileBrowser(const std::string& rootFolder) 
		: _curDirPath(std::filesystem::path(rootFolder))
	{
		_projRootPath = _curDirPath;

		_searchedString = _curDirPath.string();

		openDir(_curDirPath);

		_pathHistory.push_front(_curDirPath);

		_badPath = false;
	}



	std::optional<std::filesystem::directory_entry> display()
	{
		std::optional <std::filesystem::directory_entry> selected;

		if (ImGui::Begin("File browser"))
		{
			bool tempBadPath = _badPath;	//Since it can change with seek

			if(tempBadPath) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1., 0., 0., 1.));
			
			if (GuiBlocks::inTextStdStringHint("Path", "Search for assets", _searchedString))
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

			if (ImGui::Button("Refresh"))
				refreshContentList();

			if (ImGui::Button("Project root"))
			{
				_searchedString = _projRootPath.string();
				seek();
			}
			
			selected = printContentList();
		}

		ImGui::End();

		return selected;
	}



private:

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
		if (_pathHistory.size() >= _historySize)
			_pathHistory.pop_front();

		if (std::filesystem::is_directory(newPath))
		{
			_curDirPath = newPath;

			_searchedString = _curDirPath.string();

			_badPath = false;

			refreshContentList();
		}
		else
		{
			_badPath = true;
		}
	}



	std::optional<std::filesystem::directory_entry> printContentList()
	{
		std::optional<std::filesystem::directory_entry> result;

		if (ImGui::ListBoxHeader("Contents"))
		{
			for (const std::filesystem::directory_entry& de : _contents)
			{
				if (ImGui::Button(de.path().filename().string().data()))
				{
					if (de.is_directory())
					{
						_searchedString = de.path().string();
						seek();
					}
					else
					{
						result = de;
					}
				}
			}
			ImGui::ListBoxFooter();
		}

		return result;
	}



	void refreshContentList()
	{
		_contents.clear();

		auto file = std::filesystem::directory_entry(_curDirPath);

		if (file.exists() && file.is_directory())
		{
			std::filesystem::directory_iterator dirIter(_curDirPath);
			for (const std::filesystem::directory_entry& de : dirIter)
				_contents.push_back(de);
		}
	}
};