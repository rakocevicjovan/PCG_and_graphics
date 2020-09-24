#pragma once
#include "GUI.h"



class UIGraph
{
	static void nodeEditorWindow(const char* title)
	{
		ImGui::Begin(title);
		imnodes::BeginNodeEditor();

		imnodes::BeginNode(0);

		// Has to be added before other things in a node
		imnodes::BeginNodeTitleBar();
		ImGui::TextUnformatted("Node title");
		imnodes::EndNodeTitleBar();

		imnodes::BeginInputAttribute(0);
		ImGui::Text("Input pin.");
		imnodes::EndInputAttribute();

		imnodes::BeginOutputAttribute(0);
		ImGui::Text("Output pin.");
		imnodes::EndOutputAttribute();

		imnodes::EndNode();

		imnodes::EndNodeEditor();
		ImGui::End();
	}


};