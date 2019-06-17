#include "ToolGUI.h"

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
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("REEEEE");

		//ImGui::SliderFloat3("Light RGB", &csDef.celestial.alc.x, 0, 1);
		ImGui::SliderFloat4("Repeats", &csDef.repeat.x, 1024.f, 1024.f * 16.f);
		ImGui::InputFloat("Density coefficient", &csDef.repeat.w, 1., 5., 3);

		ImGui::SliderFloat("Light intensity", &csDef.celestial.ali, 0, 5);	//32k - 100k lux for the sun

		ImGui::InputFloat3("Extinction", &csDef.rgb_sig_absorption.x, 3);

		ImGui::SliderFloat3("Light position", &csDef.celestial.pos.x, -1000, 1000);

		ImGui::SliderFloat("HeightMask low", &csDef.heightMask.x, 100, 9000);
		ImGui::InputFloat("HeightMask high", &csDef.heightMask.y, 10, 50, 3);
		//ImGui::InputFloat2("Height mask manual", &csDef.heightMask.x, 0, 0);

		ImGui::SliderFloat("Eccentricity", &csDef.eccentricity, -1, 1);

		ImGui::SliderFloat("ScrQuadOffset", &csDef.scrQuadOffset, 0, 3);

		ImGui::SliderFloat("Global coverage", &csDef.globalCoverage, 0, 1);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		//doesn't fix automatically so I do
		csDef.heightMask.y = max(csDef.heightMask.x, csDef.heightMask.y);
	}

}