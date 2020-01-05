#pragma once
#include "IMGUI/imgui.h"
#include <vector>
#include <string>


class TDGUI
{
	ImVec2 _buildingWidgetPos;
	ImVec2 _buildingWidgetSize;

	struct BuildingGuiDef
	{
		std::string modelName;
		std::string towerDesc;
		std::string towerName;
		void* towerIcon;
	};

	std::vector<BuildingGuiDef> _buildingGuiDefs;

public:

	void addBuildingGuiDef(const std::string& modelName, const std::string& towerDesc, const std::string& towerName, void* towerIcon)
	{
		_buildingGuiDefs.push_back({modelName, towerDesc, towerName, towerIcon});
	}


	void init(ImVec2 pos, ImVec2 size)
	{
		_buildingWidgetPos = pos;
		_buildingWidgetSize = size;
	}



	bool renderBuildingWidget(std::string& structureName)
	{
		bool result = false;

		ImGui::SetNextWindowPos(_buildingWidgetPos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(_buildingWidgetSize, ImGuiCond_Once);
		ImGui::Begin("Buildings", false);

		for (int i = 0; i < _buildingGuiDefs.size(); ++i)
		{
			ImGui::PushID(i);
			
			if (ImGui::ImageButton(_buildingGuiDefs[i].towerIcon, ImVec2(64, 64)))
			{
				result = true;
				structureName = _buildingGuiDefs[i].modelName;
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::SetTooltip(_buildingGuiDefs[i].towerDesc.c_str());
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Text(_buildingGuiDefs[i].towerName.c_str());

			ImGui::PopID();
		}

		ImGui::End();

		return result;
	}
};