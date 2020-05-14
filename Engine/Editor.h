#pragma once
#include <deque>
#include "MeshDisplay.h"
#include "EditorLayout.h"
#include "Math.h"
#include "FileUtilities.h"
#include "GameObject.h"



class Editor
{
private:

	uint8_t MAX_UNDOS = 8u;

	std::string _projRoot;
	std::string _searchedString;

	std::filesystem::path _curDirPath;

	std::deque<std::filesystem::path> _pathHistory;
	std::vector<std::string> _autocomplete;

	EditorLayout _layout;
	float _w, _h;


public:



	Editor() {}



	Editor(float w, float h, const std::string& projRoot) 
		: _w(w), _h(h), _layout(w, h), _searchedString(projRoot), _projRoot(projRoot)
	{
		_curDirPath = std::filesystem::path(projRoot);

		_pathHistory.push_front(_curDirPath);

		openDir(_curDirPath);
	}



	void display(std::vector<Actor*>& actors)	//std::vector<GameObject>& objects	//std::vector<Actor*>& actors;
	{
		ImGui::SetNextWindowPos(_layout._assetListPos);
		ImGui::SetNextWindowSize(_layout._assetListSize);

		if (ImGui::Begin("Asset list"))
		{

			//for (auto& acpl : _autocomplete)
				//ImGui::Text(acpl.c_str());

			//ImGui::BeginChild("Search");

			if (ImGui::InputTextWithHint("Folder", "Search for assets", _searchedString.data(), _searchedString.capacity() + 1,
				ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue,
				[](ImGuiInputTextCallbackData* data)->int
			{
				std::string* str = static_cast<std::string*>(data->UserData);
				IM_ASSERT(data->Buf == str->data());
				str->resize(data->BufTextLen);
				data->Buf = str->data();
				return 0;
			}, &_searchedString))
			{
				seek();
			}


			if (ImGui::Button("Seek"))
			{
				seek();
			}

			ImGui::SameLine();

			if (ImGui::Button("Up"))
			{
				openDir(_curDirPath.parent_path());
			}

			ImGui::SameLine();

			if (ImGui::Button("Back"))
			{
				stepBack();
			}

			//ImGui::EndChild();


			ImGui::ListBoxHeader("Contents");

			std::filesystem::directory_iterator dirIter(_curDirPath);

			for (const std::filesystem::directory_entry& de : dirIter)	//for (int i = 0; i < _contents.size(); i++)
			{
				if (ImGui::Button(de.path().filename().string().data()))	//ImGui::Button(_contents[i].c_str())
				{
					if (de.is_directory())
						openDir(de.path());
				}
			}
			ImGui::ListBoxFooter();

		}
		ImGui::End();


		// List of objects in a level
		ImGui::SetNextWindowPos(_layout._objListPos);
		ImGui::SetNextWindowSize(_layout._objListSize);

		if (ImGui::Begin("Object list"))
		{
			ImGui::ListBoxHeader("Objeccs");

			// this won't be possible until level structure is well defined
			//for (int i = 0; i < objects.size(); i++) objects[i].display();	

			ImGui::ListBoxFooter();
		}
		ImGui::End();


		// Selected item preview 
		ImGui::SetNextWindowPos(_layout._previewPos);
		ImGui::SetNextWindowSize(_layout._previewSize);

		if (ImGui::Begin("Selected"))
		{
		
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
			ImGui::BeginTooltip();
			ImGui::Text("Invalid path");
			ImGui::EndTooltip();
		}
	}



	void openDir(const std::filesystem::path& newPath)
	{
		if (_pathHistory.size() == MAX_UNDOS)
			_pathHistory.pop_front();

		_pathHistory.push_back(_curDirPath);
		_curDirPath = newPath;

		_searchedString = _curDirPath.string();

		_autocomplete.clear();
		FileUtils::getDirContentsAsStrings(newPath.string(), _autocomplete);
	}



	bool stepBack()
	{
		if (_pathHistory.empty())
			return false;

		_curDirPath = _pathHistory.back();
		_pathHistory.pop_back();

		_searchedString = _curDirPath.string();

		_autocomplete.clear();
		FileUtils::getDirContentsAsStrings(_curDirPath.string(), _autocomplete);

		return true;
	}

};