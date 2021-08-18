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
	void Display(Component& component)
	{
		// Could we do it the everything'sfinae way instead?
		static_assert(false && "Missing Display() implementation for component");
	};

	// Forward declarations for specializations
	template<>
	void Display<CTransform>(CTransform&);

	template<>
	void Display<CParentLink>(CParentLink&);

	template<>
	void Display<CModel>(CModel&);

	template<>
	void Display<CSkModel>(CSkModel&);


	// Forward declarations for specializations
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