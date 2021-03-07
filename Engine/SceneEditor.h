#pragma once
#include "Scene.h"
#include "GuiBlocks.h"
#include "GUI.h"

struct TestComponent
{
	float x;
	float y;
};

namespace ComponentWidgets
{
	// This is a temporary fix, shouldn't be static I believe!
	template<typename Component>
	static void Display(Component& component)
	{
		// Could we do it the everything'sfinae way instead?
		static_assert(false && "Missing Display() implementation for component");
	};

	template<>
	static void Display(TestComponent& component)
	{
		ImGui::Text("TestStruct print: x: %.3f ; y: %.3f", component.x, component.y);
	}

	template<>
	static void Display(Mesh& component)
	{
		ImGui::Text("Reeee");
	}
}

class SceneEditor
{
private:

	Scene* _scene;
	entt::registry* _registry;

	entt::entity _selected = entt::null;

public:

	void init(Scene* scene)
	{
		_scene = scene;
		_registry = &_scene->_registry;

		auto e1 = _registry->create();
		_registry->emplace<TestComponent>(e1);
		_registry->emplace<Mesh>(e1);
	}

	void update()
	{
		drawHierarchy();
	}


private:

	void drawHierarchy()
	{
		ImGui::DockSpace(ImGui::GetID("Editor docker"));

		if (ImGui::Begin("Scene hierarchy", nullptr))
		{
			ImGui::BeginListBox("Entities");
			_registry->each([&](auto entity)
				{
					drawSceneNode(_registry, entity);
				});
			ImGui::EndListBox();
		}

		if (ImGui::Button("Add entity"))
		{
			_registry->create();
		}

		if (_selected != entt::null)
		{
			displayNodeProperties<TestComponent, Mesh>(_registry, _selected);
		}

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
		{
			_selected = entt::null;
		}
		

		ImGui::End();
	}


	void drawSceneNode(entt::registry* registry, entt::entity entity)
	{
		ImGuiTreeNodeFlags flags =  ImGuiTreeNodeFlags_OpenOnArrow;
		bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity), flags, "Entity %d", entity);
		
		if (ImGui::IsItemClicked())
		{
			_selected = entity;
		}

		if (open)
		{
			ImGui::TreePop();
		}
	}

	
	template <typename EditorType>
	void displayExistingComponent(entt::registry* registry, entt::entity entity)
	{
		if (registry->has<EditorType>(entity))
		{
			auto& component = registry->get<EditorType>(entity);
			ComponentWidgets::Display(component);
		}
	}
	

	template <typename... Editables>
	void displayNodeProperties(entt::registry* registry, entt::entity entity)
	{
		if (ImGui::BeginChild("Selected entity"))
		{
			((displayExistingComponent<Editables>(registry, entity)), ...);
		}
		ImGui::EndChild();
	}
	
};