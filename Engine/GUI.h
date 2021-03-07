#pragma once
#include <vector>
#include "Window.h"
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"
#include "imnodes.h"



struct GuiElement
{
	std::string title;
	std::string content;
};



class GUI
{
public:

	static void initDxWin32(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
		io.ConfigDockingWithShift = true;

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(device, context);

		imnodes::Initialize();
	}


	static void shutDown()
	{
		imnodes::Shutdown();
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}


	static void beginFrame()
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


	static void endFrame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
};