#pragma once
#include "Scene.h"
#include "GuiBlocks.h"
#include "GUI.h"
#include "CTransform.h"

struct CStaticMesh
{
	Mesh* mesh;
};

namespace ComponentEditor
{
	// This could stay here but normalize the namespace.
	template<typename Component>
	static void Display(Component& component)
	{
		// Could we do it the everything'sfinae way instead?
		static_assert(false && "Missing Display() implementation for component");
	};


	// Individual template specs CAN NOT STAY HERE! Way too dense, even if they are mere wrappers.
	template<>
	static void Display(CTransform& transform)
	{
		GuiBlocks::displayTransform(transform);
	}


	template<>
	static void Display(CParentLink& parentLink)
	{
		ImGui::Text("Parent: %d", parentLink.parent);
	}

	template<>
	static void Display(CStaticMesh& staticMesh)
	{
		GuiBlocks::displayMesh(staticMesh.mesh);
	}
}

class SceneEditor
{
private:

	Scene* _scene;
	entt::registry* _registry;

	entt::entity _selected = entt::null;

	template <typename... DisplayableComponents>
	struct DisplayComponents {};

	// Keep the declaration up here and expand on it.
	static constexpr DisplayComponents<
		CTransform,
		CParentLink,
		CStaticMesh
	> _displayComponents{};



public:

	void init(Scene* scene)
	{
		_scene = scene;
		_registry = &_scene->_registry;
	}

	void display()
	{
		if(ImGui::Begin("Editor"))
		{
			ImGui::DockSpace(ImGui::GetID("Editor docker"));

			drawEntities();

			if (_selected != entt::null)
			{
				displayNodeProperties(_registry, _selected, std::move(_displayComponents));
			}
		}

		ImGui::End();
	}


private:

	void drawEntities()
	{
		if (ImGui::Begin("Entity list", nullptr))
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
			auto entity = _registry->create();
			_registry->emplace<CParentLink>(entity, entt::to_integral(entity) - 1);
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

		/*if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
				memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
				memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
			ImGui::EndDragDropTarget();
		}*/

		bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity), flags, "Entity %d", entity);

		if (ImGui::IsItemClicked())
		{
			_selected = entity;
		}

		if (open)
		{
			ImGui::TreePop();
		}
		
		if (ImGui::BeginDragDropTarget()) {
			// Some processing...
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginDragDropSource()) {
			// Some processing...
			ImGui::EndDragDropSource();
		}
	
	}

	
	template <typename EditorType>
	void displayExistingComponent(entt::registry* registry, entt::entity entity)
	{
		if (registry->has<EditorType>(entity))
		{
			auto& component = registry->get<EditorType>(entity);
			ComponentEditor::Display(component);
		}
	}
	

	template <typename... Editables>
	void displayNodeProperties(entt::registry* registry, entt::entity entity, DisplayComponents<Editables...> a)
	{
		if (ImGui::Begin("Selected entity") && _selected != entt::null)
		{
			((displayExistingComponent<Editables>(registry, entity)), ...);

			ImGui::NewLine();

			showAddComponentPopup();
		}
		ImGui::End();
	}

	void showAddComponentPopup()
	{
		if (ImGui::BeginPopup("Component type"))
		{
			ImGui::Text("Type 1");
			ImGui::Text("Type 2");
			ImGui::EndPopup();
		}

		if (ImGui::Button("Add component"))
		{
			ImGui::OpenPopup("Component type");
		}
		
	}
	
};