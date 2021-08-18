#pragma once
#include "Scene.h"
#include "GuiBlocks.h"
#include "GUI.h"
#include "ComponentEditors.h"
#include "InputManager.h"
#include "CEntityName.h"

class SceneEditor
{
private:

	Scene* _scene{};
	entt::registry* _registry{};

	entt::entity _selected{ entt::null };
	entt::entity _toBeRenamed{ entt::null };

	const InputManager* _inputManager{};

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

	void init(Scene* scene, const InputManager* inMan)
	{
		_scene = scene;
		_registry = &_scene->_registry;
		_inputManager = inMan;
	}


	void display()
	{
		if(ImGui::Begin("Editor"))
		{
			ImGui::DockSpace(ImGui::GetID("Editor docker"));

			displayEntityDetails(_registry, _selected);

			drawEntities();
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
				uint32_t i{ 0 };
				_registry->each([&](auto entity)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, (++i % 2) ? ImVec4(1., 1., 1., 1.) : ImVec4(.8, .8, .8, 1.));
						drawEntityListItem(_registry, entity);
						ImGui::PopStyleColor();
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


	void drawEntityListItem(entt::registry* registry, entt::entity entity)
	{
		if (_toBeRenamed == entity)
		{
			auto& name = _registry->get<CEntityName>(_toBeRenamed);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
			if (ImGui::InputTextWithHint("", "Rename", name.get(), 32, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
			{
				_toBeRenamed = entt::null;
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			return;
		}

		bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity), ImGuiTreeNodeFlags_OpenOnArrow, _registry->get<CEntityName>(entity).get());

		if (ImGui::IsItemClicked())
		{
			_selected = entity;
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			_selected = entity;
			goToEntityIfLocatable(registry, entity);
		}

		// If dragging this somewhere
		if (ImGui::BeginDragDropSource())
		{
			// Some processing...
			ImGui::EndDragDropSource();
		}

		// If dragging into this
		if (ImGui::BeginDragDropTarget())
		{
			// Some processing...
			ImGui::EndDragDropTarget();
		}

		if (open)
		{
			ImGui::TreePop();
		}
	}


	void displayEntityDetails(entt::registry* registry, entt::entity& selected)
	{
		if (selected == entt::null)
		{
			return;
		}

		if (_inputManager->isKeyDown(VK_DELETE))
		{
			_registry->destroy(selected);
			selected = entt::null;
			return;
		}

		if (_inputManager->isKeyDown('R'))
		{
			_toBeRenamed = selected;
		}

		if (_inputManager->isKeyDown('F'))
		{
			goToEntityIfLocatable(registry, selected);
		}

		displayNodeProperties(_registry, _selected, _displayComponents);
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


	void goToEntityIfLocatable(entt::registry* registry, entt::entity entity)
	{
		if (_registry->has<CTransform>(entity))
		{
			// This has a lot of set up work to get right. Must have camera access too...
		}
	}
};