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

		ImGui::SliderFloat3("Light RGB", &csDef.celestial.alc.x, 0, 1);
		
		ImGui::SliderFloat("Light intensity", &csDef.celestial.ali, 0, 100000);	//32k - 100k lux for the sun

		ImGui::SliderFloat3("Light position", &csDef.celestial.pos.x, -1000, 1000);

		ImGui::SliderFloat("HeightMask low", &csDef.heightMask.x, 100, 9000);
		ImGui::SliderFloat("HeightMask high", &csDef.heightMask.y, csDef.heightMask.x, 9000);
		//ImGui::InputFloat2("Height mask manual", &csDef.heightMask.x, 0, 0);

		ImGui::SliderFloat("Eccentricity", &csDef.eccentricity, -1, 1);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		//doesn't fix automatically so I do
		csDef.heightMask.y = max(csDef.heightMask.x, csDef.heightMask.y);
	}

}