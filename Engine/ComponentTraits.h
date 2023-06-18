#pragma once

#include "CParentLink.h"
#include "CTransform.h"
#include "CModel.h"
#include "CSkModel.h"


namespace ComponentTraits
{
	// Widgets for component previews
	template<typename Component>
	void Display(Component& component)
	{
		// Could we do it the everything'sfinae way instead?
		//static_assert(false && "Missing Display() specialization for component");
		//ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Editor display not implemented for this component type.");
	};

	template<>
	void Display<CTransform>(CTransform&);

	template<>
	void Display<CParentLink>(CParentLink&);

	template<>
	void Display<CModel>(CModel&);

	template<>
	void Display<CSkModel>(CSkModel&);


	// Component name, used by the editor
	template<typename Component>
	const char* GetComponentTypeName()
	{
		return "undefined type";
	}

	template<>
	const char* GetComponentTypeName<CTransform>();

	template<>
	const char* GetComponentTypeName<CParentLink>();

	template<>
	const char* GetComponentTypeName<CModel>();

	template<>
	const char* GetComponentTypeName<CSkModel>();
}