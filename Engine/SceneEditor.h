#pragma once
#include "Scene.h"
#include "GuiBlocks.h"
#include "GUI.h"
#include "ComponentEditors.h"


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
		CModel,
		CSkModel
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
				displayNodeProperties(_registry, _selected, _displayComponents);
			}
		}

		ImGui::End();
	}


private:

	void drawEntities()
	{
		if (ImGui::Begin("Entity list", nullptr))
		{
			if (ImGui::BeginListBox("Entities"))
			{
				_registry->each([&](auto entity)
					{
						drawSceneNode(_registry, entity);
					});
				ImGui::EndListBox();
			}
		}

		if (ImGui::Button("Add entity"))
		{
			auto entity = _registry->create();
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
		
		if (ImGui::BeginDragDropTarget()) {
			// Some processing...
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginDragDropSource()) {
			// Some processing...
			ImGui::EndDragDropSource();
		}
	}


	template <typename Editable>
	void displayExistingComponent(entt::registry* registry, entt::entity entity)
	{
		if (registry->has<Editable>(entity))
		{
			if (ImGui::TreeNode(ComponentEditor::GetComponentTypeName<Editable>()))
			{
				auto& component = registry->get<Editable>(entity);
				ComponentEditor::Display(component);
				ImGui::TreePop();
			}
		}
	}
	

	template <typename... Editables>
	void displayNodeProperties(entt::registry* registry, entt::entity entity, DisplayComponents<Editables...> a)
	{
		if (ImGui::Begin("Selected entity") && _selected != entt::null)
		{
			((displayExistingComponent<Editables>(registry, entity)), ...);

			ImGui::NewLine();

			showAddComponentPopup<Editables...>(registry, entity);
		}
		ImGui::End();
	}


	template<typename Editable>
	void showAddComponent(entt::registry* registry, entt::entity entity)
	{
		if (!registry->has<Editable>(entity))
		{
			char buf[128];
			sprintf(buf, "Add %s", ComponentEditor::GetComponentTypeName<Editable>());
			if (ImGui::Button(buf))
			{
				registry->emplace<Editable>(entity);
				ImGui::CloseCurrentPopup();
			}
		}
	}

	template <typename... Editables>
	void showAddComponentPopup(entt::registry* registry, entt::entity entity)
	{
		if (ImGui::Button("Add component"))
		{
			ImGui::OpenPopup("Select type to add");
		}

		if (ImGui::BeginPopup("Select type to add"))
		{
			((showAddComponent<Editables>(registry, entity)), ...);
			ImGui::EndPopup();
		}
	}

};