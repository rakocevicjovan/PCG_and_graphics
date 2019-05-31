#pragma once
#include "CloudscapeDefinition.h"

namespace Strife
{

	class ToolGUI
	{
	public:
		ToolGUI();
		~ToolGUI();

		static void Render(CloudscapeDefinition& csDef);
	};

}