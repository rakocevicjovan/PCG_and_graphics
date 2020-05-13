#pragma once
#include "MeshDisplay.h"
#include "EditorLayout.h"
#include "Math.h"
#include "FileUtilities.h"

class Editor
{
private:

	std::string _projRoot;

	std::string _curDirPath;
	std::vector<std::string> _contents;

	EditorLayout _layout;
	float _w, _h;

public:

	Editor() {}

	Editor(float w, float h, const std::string& projRoot) 
		: _w(w), _h(h), _layout(w, h), _curDirPath(projRoot), _projRoot(projRoot)
	{}



	void display()
	{
		ImGui::SetNextWindowPos(_layout._assetListPos);
		ImGui::SetNextWindowSize(_layout._assetListSize);

		ImGui::Begin("Asset list");

		
		ImGui::InputTextWithHint("Folder", "ayy lmao", _curDirPath.data(), _curDirPath.capacity() + 1, 
			ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue,
			[](ImGuiInputTextCallbackData* data)->int
				{
					std::string* str = static_cast<std::string*>(data->UserData);
					IM_ASSERT(data->Buf == str->data());
					str->resize(data->BufTextLen);
					data->Buf = str->data();	
					return 0; 
				},
			&_curDirPath
		);

		if (ImGui::Button("Seek"))
		{
			_contents.clear();
			FileUtils::getFolderContents(_curDirPath, _contents);
		}

		ImGui::SameLine();

		if (ImGui::Button("Back"))
		{
			_curDirPath = _curDirPath.substr(0., _curDirPath.find_last_of("\\/") + 1);
			_contents.clear();
			FileUtils::getFolderContents(_curDirPath, _contents);
		}


		ImGui::ListBoxHeader("Contents");

		for (int i = 0; i < _contents.size(); i++)
		{
			//ImGui::Text(_contents[i].c_str());
			if (ImGui::Button(_contents[i].c_str()))
			{
				_curDirPath.append(_contents[i]);
				_contents.clear();
				FileUtils::getFolderContents(_curDirPath, _contents);
			}
		}

		ImGui::ListBoxFooter();

		ImGui::End();


		ImGui::SetNextWindowPos(_layout._objListPos);
		ImGui::SetNextWindowSize(_layout._objListSize);

		ImGui::Begin("Object list");
		ImGui::ListBoxHeader("Objeccs");
		ImGui::ListBoxFooter();
		ImGui::End();


		ImGui::SetNextWindowPos(_layout._previewPos);
		ImGui::SetNextWindowSize(_layout._previewSize);

		ImGui::Begin("Preview selected");
		ImGui::End();
	}

};