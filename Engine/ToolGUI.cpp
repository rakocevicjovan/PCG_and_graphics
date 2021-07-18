#include "pch.h"
#include "ToolGUI.h"
#include "GUI.h"
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"

namespace Strife
{

	ToolGUI::ToolGUI()
	{
	}


	ToolGUI::~ToolGUI()
	{
	}



	void ToolGUI::Render(CloudscapeDefinition& csDef)
	{
		GUI::beginFrame();

		ImGui::Begin("Cloud rendering control panel");

		/* volume */
		ImGui::BeginGroup();
		ImGui::Text("Modelling parameters");
		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		ImGui::InputFloat("Planet's radius",		&csDef.planetRadius, 1000, 2000);
		ImGui::SliderFloat("Layer_bot",				&csDef.heightMask.x, 100, 9000);
		ImGui::InputFloat("Layer_top",				&csDef.heightMask.y, 10, 50);

		ImGui::SliderFloat("Base repeat",			&csDef.repeat.x, 1024.f, 1024.f * 16.f);
		ImGui::SliderFloat("Fine repeat",			&csDef.repeat.y, 16.f, 256.f);
		ImGui::SliderFloat("Texture span",			&csDef.textureSpan, 0.f, 1.f);
		ImGui::InputFloat("Density multiplier",		&csDef.repeat.w, 1., 5.);
		ImGui::SliderFloat("Carving threshold",		&csDef.carvingThreshold, -.01f, 1.01f);

		ImGui::SliderFloat("Global coverage",		&csDef.globalCoverage, 0.01f, .99f);
		ImGui::SliderFloat("View distance",			&csDef.distanceLimit, 5000.f, 50000.f);
		ImGui::SliderFloat("ScrQuadOffset",			&csDef.scrQuadOffset, 0, 3);

		ImGui::EndGroup();
		/* volume */

		ImGui::Dummy(ImVec2(0.0f, 25.0f));

		/* illumination */
		ImGui::BeginGroup();
		ImGui::Text("Illumination parameters");
		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		ImGui::InputFloat("Light intensity",	&csDef.celestial.ali, 1, 10);
		ImGui::SliderFloat3("Light position",	&csDef.celestial.pos.x, -1000, 1000);
		ImGui::InputFloat3("Extinction",		&csDef.rgb_sig_absorption.x);
		ImGui::SliderFloat("Eccentricity",		&csDef.eccentricity, -1, 1);
		ImGui::InputFloat3("Sky colour",		&csDef.skyRGB.x, "%.4f");
		ImGui::InputFloat3("Ambient top",		&csDef.ALTop.x, "%.4f");
		ImGui::InputFloat3("Ambient bot",		&csDef.ALBot.x, "%.4f");

		ImGui::EndGroup();
		/* illumination */

		ImGui::End();

		GUI::endFrame();

		//doesn't fix automatically so I do
		csDef.heightMask.y = std::max(csDef.heightMask.x, csDef.heightMask.y);
	}

}

//ImGui::ColorPicker3("Light colour", &csDef.celestial.alc.x);	//ImGui::InputFloat3("Light colour", &csDef.celestial.alc.x, 3);