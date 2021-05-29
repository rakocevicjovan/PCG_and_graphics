#pragma once
#include "Level.h"
#include "AssImport.h"
#include "Engine.h"
#include "Scene.h"
#include "GUI.h"
#include "SkeletalModelInstance.h"
#include "Sampler.h"
#include "ShaderManager.h"
#include <memory>



class AssimpLoader : public Level
{
private:
	// Temporarily here, pass them as ptr/ref from the editor to feed into that one
	ShaderManager _shMan;
	FileBrowser _fileBrowser;

	Scene _scene;

	std::vector<std::unique_ptr<AssImport>> _previews;
	AssImport* _curPreview{ nullptr };

	// For previewing models in 3d
	PointLight _pointLight;
	CBuffer _pointLightCB;

	// Floor 
	std::unique_ptr<Mesh> _floorMesh;

	RenderTarget _renderTarget;

public:

	AssimpLoader(Engine& sys) : 
		Level(sys), 
		_scene(sys, AABB(SVec3(), SVec3(500.f * .5)), 5),
		_fileBrowser("C:\\Users\\metal\\source\\repos\\PCG_and_graphics_stale_memes\\Models\\Animated")
	{
		_renderTarget = RenderTarget(S_DEVICE, 1024, 1024, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE);
	}

	

	//void desiredApi()
	//{
	//	Scene scene(_sys, AABB(SVec3(), SVec3(300)));

	//	// init floor
	//	scene.insert(floor)

	//	//init light
	//	scene.insert(light)

	//	//Show scene UI to allow adding stuff at will
	//}



	void init(Engine& sys) override
	{
		// Move later
		_shMan.init(S_DEVICE, &sys._shaderCache);
		_shMan.loadExistingKeys(NATURAL_COMPS);

		// Create point light for scene preview
		LightData ld = LightData(SVec3(1.), .2, SVec3(1.), .6, SVec3(1.), .7);

		_pointLight = PointLight(ld, SVec4(0., 300., -300., 1.));

		_pointLight.createCBuffer(S_DEVICE, _pointLightCB);
		_pointLight.updateCBuffer(S_CONTEXT, _pointLightCB);
		_pointLight.bind(S_CONTEXT, _pointLightCB);

		float _tSize = 500.f;
		Procedural::Terrain terrain(2, 2, SVec3(_tSize), SVec3(-_tSize * .5f, -0.f, -_tSize * .5f));

		_floorMesh = std::make_unique<Mesh>(terrain, S_DEVICE);
		auto shPack = _shMan.getShaderAuto(_floorMesh->_vertSig, _floorMesh->_material.get());
		_floorMesh->_material->setVS(shPack->vs);
		_floorMesh->_material->setPS(shPack->ps);
	}



	void update(const RenderContext& rc) override {}



	void draw(const RenderContext& rc) override
	{
		//DirectX::XMMatrixPerspectiveLH(1024, 1024, 1., 1000.); this seems... bugged? Or I don't understand the params well?

		S_RANDY._cam.SetProjectionMatrix(DirectX::XMMatrixPerspectiveFovLH(PI / 3.0f, 1.f, 1., 1000.));
		S_RANDY.updatePerCamBuffer(1024, 1024);

		Viewport viewport(1024, 1024);
		viewport.bind(rc.d3d->getContext());
		_renderTarget.bind(rc.d3d->getContext());
		_renderTarget.clear(rc.d3d->getContext());

		_floorMesh->draw(rc.d3d->getContext());

		S_RANDY.d3d()->TurnOffAlphaBlending();
		
		if (_curPreview)
		{
			_curPreview->draw(S_CONTEXT, rc.dTime);
		}

		SMatrix oldProjMat = S_RANDY._cam.GetProjectionMatrix();
		SMatrix projMat = DirectX::XMMatrixPerspectiveFovLH(PI / 3.0f, 2560. / 1440., 1., 1000.);
		rc.cam->SetProjectionMatrix(projMat);
		S_RANDY.updatePerCamBuffer(2560, 1440);

		_sys._renderer.setDefaultRenderTarget();

		GUI::beginFrame();
		drawUI();
		GUI::endFrame();

		rc.d3d->EndScene();
	}


	void drawUI()
	{
		auto selected = _fileBrowser.display();

		if (selected.has_value())
		{
			auto& selectedPath = selected.value();

			if (!alreadyLoaded(selectedPath))
			{
				_previews.push_back(std::make_unique<AssImport>());

				if (!_previews.back()->loadAiScene(S_DEVICE, selectedPath.path().string().c_str(), "C:\\Users\\metal\\Desktop\\AeonTest\\", &_shMan))
				{
					_previews.pop_back();
				}
			}
		}

		if (ImGui::Begin("Content"))
		{
			ImGui::BeginTabBar("Loaded scenes", ImGuiTabBarFlags_AutoSelectNewTabs);
			ImGui::NewLine();

			for (UINT i = 0; i < _previews.size(); i++)
			{
				std::string sceneName = _previews[i]->getPath().filename().string();
				if (ImGui::BeginTabItem(sceneName.c_str()))
				{
					_curPreview = _previews[i].get();

					if (!_previews[i]->displayPreview(sceneName))
					{
						_previews.erase(_previews.begin() + i);
						_curPreview = nullptr;
					}

					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
		ImGui::End();

		if (ImGui::Begin("Preview window"))
		{
			// Make it scale with aspect ratio of render target
			SVec2 rtSize{ _renderTarget.size().first, _renderTarget.size().second };
			SVec2 windowSize{ *reinterpret_cast<SVec2*>(&ImGui::GetWindowSize()) };
			SVec2 newSize = Math::resizeRetainAspectRatio(rtSize, windowSize);
			ImVec2 aspectRatioCorrectImageSize = ImVec2(newSize.x, newSize.y);
			ImGui::SetCursorPos((ImGui::GetWindowSize() - aspectRatioCorrectImageSize) * 0.5f);
			ImGui::Image(_renderTarget.asSrv(), aspectRatioCorrectImageSize);
		}
		ImGui::End();
	}



	bool alreadyLoaded(const std::filesystem::directory_entry& selected)
	{
		for (auto& p : _previews)
		{
			if (p->getPath() == selected.path())
			{
				return true;
			}
		}
		return false;
	}
};