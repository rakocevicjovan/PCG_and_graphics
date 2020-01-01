#pragma once
#include "IMGUI/imgui.h"
#include <vector>
#include <string>


class TDGUI
{
	ImVec2 _buildingWidgetPos;
	ImVec2 _buildingWidgetSize;

	struct TowerGuiDef
	{
		std::string towerDesc;
		std::string towerName;
		void* towerIcon;
	};

	std::vector<TowerGuiDef> _towerGuiDefs;

public:

	void addTowerGuiDef(const std::string& towerDesc, const std::string& towerName, void* towerIcon)
	{
		_towerGuiDefs.push_back({towerDesc, towerName, towerIcon});
	}

	void init(ImVec2 pos, ImVec2 size)
	{
		_buildingWidgetPos = pos;
		_buildingWidgetSize = size;
	}



	void renderBuildingWidget(bool& building, int& structureIndex)
	{
		ImGui::SetNextWindowPos(_buildingWidgetPos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(_buildingWidgetSize, ImGuiCond_Once);
		ImGui::Begin("Buildings", false);

		ImGui::Text("Buildings");
		ImGui::ListBoxHeader("");

		for (int i = 0; i < _towerGuiDefs.size(); ++i)
		{
			ImGui::PushID(i);
			
			
			if (ImGui::ImageButton(_towerGuiDefs[i].towerIcon, ImVec2(64, 64)))
			{
				building = true;
				structureIndex = i;
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::SetTooltip(_towerGuiDefs[i].towerDesc.c_str());
				ImGui::EndTooltip();
			}
			
			ImGui::Text(_towerGuiDefs[i].towerName.c_str());
			ImGui::PopID();
		}

		ImGui::ListBoxFooter();

		ImGui::End();
	}
};