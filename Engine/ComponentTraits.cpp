#include "pch.h"

#include "ComponentTraits.h"
#include "GuiBlocks.h"
#include "AssetViews.h"


namespace ComponentTraits
{

	// Individual template specs for editor display... I believe it's cleaner to keep them in one file.
	template<>
	void Display(CTransform& transform)
	{
		ImGui::PushID(&transform);
		GuiBlocks::displayTransform(transform);
		ImGui::PopID();
	}


	template<>
	void Display(CParentLink& parentLink)
	{
		ImGui::Text("Parent: %d", parentLink.parent);
	}


	template<>
	void Display(CModel& model)
	{
		if (!model.model)
		{
			ImGui::Text("No model assigned.");
			return;
		}

		ImGui::Indent();

		for (auto& mesh : model.model->_meshes)
		{
			GuiBlocks::displayMesh(&mesh);
		}

		ImGui::Unindent();
	}


	template<>
	void Display(CSkModel& skModel)
	{
		if (!skModel.skModel)
		{
			ImGui::Text("No model assigned.");
			return;
		}

		ImGui::Indent();

		AssetViews::PrintSkModel(skModel.skModel);

		ImGui::Unindent();
	}


	template<>
	const char* GetComponentTypeName<CTransform>()
	{
		return "transform component";
	}

	template<>
	const char* GetComponentTypeName<CParentLink>()
	{
		return "parent link component";
	}

	template<>
	const char* GetComponentTypeName<CModel>()
	{
		return "static model component";
	}

	template<>
	const char* GetComponentTypeName<CSkModel>()
	{
		return "skeletal model component";
	}
}