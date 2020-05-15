#include "GameObject.h"
#include "MeshDisplay.h"



static void displayRenderable(Renderable& r)
{
	ImGui::Text("Local transform: ");
	ImGui::PushID("lt");
	displayTransform(r._localTransform);
	ImGui::PopID();

	ImGui::Text("Transform: ");
	ImGui::PushID("gt");
	displayTransform(r._transform);
	ImGui::PopID();

	ImGui::Text("Mesh: ");
	ImGui::PushID("mesh");
	displayMesh(*r.mesh);
	ImGui::PopID();

	ImGui::Text("Material");
	displayMaterial(*r.mat);

	ImGui::Separator();
}


static void displayActor(Actor& a)
{
	//std::string actName("Actor nr. ");
	//actName += std::to_string(i);


	if (ImGui::BeginChild("Actor"))
	{
		ImGui::BeginGroup();

		ImGui::Text("Actor: ");

		ImGui::Text("Transform: ");
		displayTransform(a._transform);

		for (int i = 0; i < a._renderables.size(); i++)
		{
			ImGui::PushID(i);
			ImGui::Text("Renderable: ");
			displayRenderable(a._renderables[i]);
			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::EndGroup();
	}
	ImGui::EndChild();
}