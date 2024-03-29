#pragma once


// This code is valid at the time of commenting. Should be kept until a decision is made.
// The editor can live on top of the engine or inside the engine - this is a choice.

#include "Engine.h"
#include "FileBrowser.h"
#include "SceneEditor.h"
#include "FPSCounter.h"

//#include "EditorLayout.h"

class Editor
{
private:

	Engine& _engine;
	SceneEditor _sceneEditor;
	FileBrowser _fileBrowser;
	std::unique_ptr<Project> _project;

public:

	Editor(Engine& engine) : _engine(engine)
	{

	}


	void display(Scene* scene)
	{
		GUI::BeginFrame();

		if (ImGui::Begin("Aeolian"))
		{
			_sceneEditor.display();

			//ImGui::Image();
			_fileBrowser.display();

			displayStats(scene, _engine._fpsCounter);
		}
		ImGui::End();

		GUI::EndFrame();
	}


	void displayStats(Scene* scene, FPSCounter& fpsCounter)
	{
		if (ImGui::Begin("Stats"))
		{
			if (ImGui::Begin("FPS"))
			{
				ImGui::Text("FPS: %.2f", fpsCounter.getAverageFPS());
				ImGui::Text("Ms:  %.6f", fpsCounter.getAverageFrameTime());
			}

			ImGui::End();

			if (ImGui::Begin("Collision"))
			{
				ImGui::Text("Octree nodes: %d", scene->_octree.getNodeCount());
				ImGui::Text("Octree hulls: %d", scene->_octree.getHullCount());
			}
			ImGui::End();

			if (ImGui::Begin("Culling"))
			{
				ImGui::Text("Culled objects: %d", scene->_numCulled);
			}
			ImGui::End();
		}
		ImGui::End();
	}
};