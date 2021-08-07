#include "pch.h"

#include "ComponentEditors.h"
#include "GuiBlocks.h"


namespace ComponentEditor
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
		if (!skModel._skModel)
		{
			ImGui::Text("No model assigned.");
			return;
		}

		ImGui::Indent();

		for (auto& mesh : skModel._skModel->_meshes)
		{
			GuiBlocks::displayMesh(&mesh);
		}

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