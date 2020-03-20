#pragma once
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"
#include <string>
#include <d3d11.h>

class GUI
{

public:

	static void drawTextureInWindow(float px, float py, float iw, float ih, const std::string& title, ID3D11ShaderResourceView* srv)
	{
		ImGui::SetNextWindowPos(ImVec2(px, py), ImGuiCond_Once);
		//ImGui::SetNextWindowSize(ImVec2(iw, ih), ImGuiCond_Once);

		ImGui::Begin(title.c_str(), false);

		ImGui::Image(srv, ImVec2(iw, ih));

		ImGui::End();
	}

};