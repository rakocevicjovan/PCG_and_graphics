#pragma once
#include <vector>
#include "Window.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_internal.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"
#include "Gizmo.h"
#include "imnodes.h"
#include "FontHeaders/IconsFontAwesome4.h"



struct GuiElement
{
	std::string title;
	std::string content;
};



class GUI
{
public:

	static void InitDxWin32(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
		io.ConfigDockingWithShift = true;
		io.WantCaptureMouse = true;

		io.Fonts->AddFontDefault();
		
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromFileTTF("../../../assets/Fonts/" FONT_ICON_FILE_NAME_FA, 16.0f, &icons_config, icons_ranges);
		// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid


		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(device, context);

		imnodes::Initialize();
		Gizmo::Initialize();
	}


	static void ShutDown()
	{
		imnodes::Shutdown();
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}


	static void BeginFrame()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		Gizmo::BeginFrame();
	}


	static void EndFrame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}


	static void RenderGuiElems(const std::vector<GuiElement>& elements)
	{
		for (const GuiElement& e : elements)
		{
			ImGui::Begin(e.title.c_str());
			ImGui::Text(e.content.c_str());
			ImGui::End();
		}
	}


	template <typename GuiCodeFunc, typename... Args>
	static void RenderInWindow(const char* title, const GuiCodeFunc& lambda, Args...)
	{
		ImGui::Begin(title);
		lambda(Args...);
		ImGui::End();
	}
};