#include "GameObject.h"
#include "MeshDisplay.h"



static void displayRenderable(Renderable& r)
{
	ImGui::Text("Renderable");

	ImGui::Text("Local transform");
	displayTransform(r._localTransform);

	ImGui::Text("Global transform");
	displayTransform(r._transform);

	ImGui::Text("Mesh");
	displayMesh(*r.mesh);

	ImGui::Text("Material");
	displayMaterial(*r.mat);

	ImGui::Separator();
}


static void displayActor(Actor& a)
{
	ImGui::BeginChild("Actor##");

	ImGui::Text("Actor: ");

	displayTransform(a._transform);

	for (int i = 0; i < a._renderables.size(); i++)
	{
		displayRenderable(a._renderables[i]);
	}

	ImGui::Separator();
	ImGui::EndChild();
}