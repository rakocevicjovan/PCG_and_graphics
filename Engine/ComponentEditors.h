#pragma once
#include "CParentLink.h"
#include "CStaticMesh.h"
#include "CTransform.h"

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
	static void Display(CStaticMesh& staticMesh)
	{
		if (!staticMesh.model)
		{
			ImGui::Text("No model assigned.");
			return;
		}

		ImGui::Indent();

		for (auto& mesh : staticMesh.model->_meshes)
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
	static const char* GetComponentTypeName<CStaticMesh>()
	{
		return "static mesh component";
	}

}