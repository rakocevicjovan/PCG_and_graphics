#pragma once
#include "GUI.h"
#include "FileBrowser.h"
#include "RenderTarget.h"

class ImportTool
{
public:

	FileBrowser _fileBrowser;
	RenderTarget _renderTarget;

	void init(ID3D11Device* device, const std::string& importPath)
	{
		_fileBrowser = FileBrowser(importPath);
		_renderTarget = RenderTarget(device, 1024, 1024, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE);
	}

	void display()
	{
		if(ImGui::Begin("Import tool"))
		{

			ImGui::Image(_renderTarget., ImVec2(300, 300));

			_fileBrowser.display();
		}
		ImGui::End();
	}
};