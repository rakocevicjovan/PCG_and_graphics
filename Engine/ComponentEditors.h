#pragma once
#include "CParentLink.h"
#include "CTransform.h"
#include "CModel.h"
#include "CSkModel.h"

namespace ComponentEditor
{
	// Displaying
	// This could stay here but normalize the namespace.
	template<typename Component>
	static void Display(Component& component)
	{
		// Could we do it the everything'sfinae way instead?
		static_assert(false && "Missing Display() implementation for component");
	};

	// Individual template specs for editor display... I believe it's cleaner to keep them in one file.
	template<>
	static void Display(CTransform& transform)
	{
		ImGui::PushID(&transform);
		GuiBlocks::displayTransform(transform);
		ImGui::PopID();
	}

	template<>
	static void Display(CParentLink& parentLink)
	{
		ImGui::Text("Parent: %d", parentLink.parent);
	}

	template<>
	static void Display(CModel& model)
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
	static void Display(CSkModel& skModel)
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


	// Just a name
	template<typename Component>
	static const char* GetComponentTypeName()
	{
		return "";
	}

	template<>
	static const char* GetComponentTypeName<CTransform>()
	{
		return "transform component";
	}

	template<>
	static const char* GetComponentTypeName<CParentLink>()
	{
		return "parent link component";
	}

	template<>
	static const char* GetComponentTypeName<CModel>()
	{
		return "static model component";
	}

	template<>
	static const char* GetComponentTypeName<CSkModel>()
	{
		return "skeletal model component";
	}

}