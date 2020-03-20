#pragma once
#include <vector>
#include <d3d11.h>
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"



struct GuiElement
{
	std::string title;
	std::string content;
};



class GUI
{
public:

	static void startGuiFrame()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}


	//convenience function for fast windows
	static void renderGuiElems(const std::vector<GuiElement>& elements)
	{
		for (const GuiElement& e : elements)
		{
			ImGui::Begin(e.title.c_str());
			ImGui::Text(e.content.c_str());
			ImGui::End();
		}
	}


	static void endGuiFrame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}


	// Useful for outputting debug textures
	static void drawImagePanel(float pX, float pY, float sX, float sY, std::string& title, ID3D11ShaderResourceView* srv)
	{
		ImGui::SetNextWindowPos(ImVec2(pX, pY), ImGuiCond_Once);
		//ImGui::SetNextWindowSize(ImVec2(sX, sY), ImGuiCond_Once);

		ImGui::Begin(title.c_str(), false);
		ImGui::Image(srv, ImVec2(sX, sY));
		ImGui::End();
	}



};