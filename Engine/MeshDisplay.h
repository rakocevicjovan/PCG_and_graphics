#pragma once
#include "Mesh.h"
#include "IMGUI/imgui.h"
#include <string>
#include <sstream>
#include <algorithm>

static void displayVertex(Vert3D& v)
{
	ImGui::Text("P: %.2f, %.2f, %.2f", v.pos.x, v.pos.y, v.pos.z);
	ImGui::SameLine();
	ImGui::Text("T: %.2f, %.2f", v.texCoords.x, v.texCoords.y);
	ImGui::SameLine();
	ImGui::Text("N: %.2f, %.2f, %.2f", v.normal.x, v.normal.y, v.normal.z);
}



static void displayTransform(SMatrix& m)
{
	ImGui::Text("Transform");

	ImGui::InputFloat4("X", &m.m[0][0], 2);
	ImGui::InputFloat4("Y", &m.m[1][0], 2);
	ImGui::InputFloat4("Z", &m.m[2][0], 2);
	ImGui::InputFloat4("T", &m.m[3][0], 2);

	ImGui::Separator();
}



static void displayMaterial(Material& mat)
{
	ImGui::Text("Shaders: ");
	std::string vsName(mat.getVS()->_path.begin(), mat.getVS()->_path.end());
	ImGui::Text(vsName.c_str());

	std::string psName(mat.getPS()->_path.begin(), mat.getPS()->_path.end());
	ImGui::Text(psName.c_str());

	ImGui::Text("Textures");
	for (int i = 0; i < mat._texDescription.size(); i++)
	{
		ImGui::Text(TEX_ROLE_MAP.at(mat._texDescription[i]._role));
		ImGui::SameLine();
		ImGui::Text(mat._texDescription[i]._tex->getName().c_str());
	}

	ImGui::Text("Opaque: " + mat._opaque);
}



static void displayMesh(Mesh& mesh, bool dVerts = 0, bool dInds = 0)
{
	ImGui::Text("Mesh");

	// Vertices
	if (dVerts)
	{
		ImGui::ListBoxHeader("Vertices");

		for (Vert3D& v : mesh._vertices)
		{
			displayVertex(v);
			ImGui::Separator();
		}

		ImGui::ListBoxFooter();
		ImGui::Separator();
	}


	// Indices
	if (dInds)
	{
		ImGui::Text("Indices: ");
		std::stringstream result;
		std::copy(mesh._indices.begin(), mesh._indices.end(), std::ostream_iterator<int>(result, " "));
		ImGui::TextWrapped(result.str().c_str());
		ImGui::Separator();
	}

	displayTransform(mesh._transform);

	ImGui::Separator();
}