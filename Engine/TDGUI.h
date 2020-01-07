#pragma once
#include "IMGUI/imgui.h"
#include "BuildingGuiDef.h"
#include <vector>
#include <string>
#include <map>


struct Widget
{
	ImVec2 _pos;
	ImVec2 _size;
	bool _visible;

	Widget() {};

	Widget(const ImVec2& pos, const ImVec2& size, bool visible = true)
		: _pos(pos), _size(size), _visible(visible) {}
};


struct TransientWidget : public Widget
{
	float _age;
	float _lifetime;

	TransientWidget(const ImVec2& pos, const ImVec2& size, float duration, bool visible = true)
		: Widget(pos, size, visible), _lifetime(duration), _age(0.f) {}
};


class TDGUI
{
	std::vector<BuildingGuiDef> _buildingGuiDefs;

	std::map<std::string, Widget*> _widgets;
	std::map<std::string, TransientWidget*> _transients;

	Widget _paletteWidget;

public:

	void addBuildingGuiDef(const std::string& towerDesc, const std::string& towerName, void* towerIcon)
	{
		_buildingGuiDefs.push_back({towerDesc, towerName, towerIcon});
	}


	void addBuildingGuiDef(const BuildingGuiDef& guiDef)
	{
		_buildingGuiDefs.push_back(guiDef);
	}


	void init(const ImVec2& pos, const ImVec2& size)
	{
		_paletteWidget._pos = pos;
		_paletteWidget._size = size;
	}


	bool renderBuildingPalette(UINT& index) const
	{
		bool result = false;

		ImGui::SetNextWindowPos(_paletteWidget._pos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(_paletteWidget._size, ImGuiCond_Once);

		ImGui::Begin("Buildings", false);

		for (int i = 0; i < _buildingGuiDefs.size(); ++i)
		{
			ImGui::PushID(i);
			
			if (ImGui::ImageButton(_buildingGuiDefs[i]._icon, ImVec2(64, 64)))
			{
				result = true;
				index = i;
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::SetTooltip(_buildingGuiDefs[i]._desc.c_str());
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Text(_buildingGuiDefs[i]._name.c_str());

			ImGui::PopID();
		}

		ImGui::End();

		return result;
	}


	bool renderSelectedWidget(const BuildingGuiDef& def) const
	{
		bool result = false;

		ImGui::SetNextWindowPos(_widgets.at("selected")->_pos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(_widgets.at("selected")->_size, ImGuiCond_Once);
		
		ImGui::Begin("Selected", false);

		ImGui::Image(def._icon, ImVec2(128, 128));
		ImGui::Text(def._name.c_str());
		ImGui::Text(def._desc.c_str());

		result = ImGui::Button("DELET DIS");

		ImGui::End();

		return result;
	}


	void makeTransientWidget()
	{
		//_transients.push_back();
	}


	//allows creating widgets on the fly, some are static to avoid map searches as they always exist
	bool createWidget(const ImVec2& pos, const ImVec2& size, const std::string& name)
	{
		return _widgets.emplace(name, new Widget(pos, size)).second;
	}


	bool deleteWidget(const std::string& name)
	{
		auto iter = _widgets.find(name);
		
		if (iter == _widgets.end())
			return false;

		delete iter->second;

		return (_widgets.erase(name) == 1);
	}
};