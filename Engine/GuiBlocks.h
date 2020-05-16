#pragma once
#include "Mesh.h"
#include "GameObject.h"
#include "IMGUI/imgui.h"
#include <d3d11.h>
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
	ImGui::InputFloat4("X##", &m.m[0][0], 2);
	ImGui::InputFloat4("Y##", &m.m[1][0], 2);
	ImGui::InputFloat4("Z##", &m.m[2][0], 2);
	ImGui::InputFloat4("T##", &m.m[3][0], 2);
	ImGui::Separator();
}



static void displayTexture(ID3D11ShaderResourceView* texSrv, const std::string& path, float w, float h, int n)
{
	//inTextStdString("Path: ", path);

	ImGui::Text(path.c_str());

	char buff[50];
	sprintf(buff, "Width: %i \nHeight: %i \nChannels: %i", 
		static_cast<int>(w), static_cast<int>(h), static_cast<int>(n));

	ImGui::Text(buff);

	if (texSrv != nullptr)
	{
		ImGui::Image(texSrv, ImVec2(512., 512.));
	}
}



static void displayMaterial(Material& mat)
{
	ImGui::BeginGroup();

	ImGui::Text("Shaders: ");

	VertexShader* vs = mat.getVS();
	if (vs)
	{
		std::string vsName(vs->_path.begin(), vs->_path.end());
		ImGui::Text(vsName.c_str());
	}
	else
	{
		ImGui::Text("None loaded");
	}

	PixelShader* ps = mat.getPS();
	if (ps)
	{
		std::string psName(ps->_path.begin(), ps->_path.end());
		ImGui::Text(psName.c_str());
	}
	else
	{
		ImGui::Text("None loaded");
	}

	ImGui::Separator();

	ImGui::Text("Textures");
	for (int i = 0; i < mat._texDescription.size(); i++)
	{
		ImGui::Text(TEX_ROLE_MAP.at(mat._texDescription[i]._role));
		ImGui::SameLine();
		ImGui::Text(mat._texDescription[i]._tex->getName().c_str());
	}

	ImGui::Separator();

	ImGui::Text(std::string("Opaque: " + std::to_string(mat._opaque)).c_str());

	ImGui::EndGroup();
}



static void displayMesh(Mesh& mesh, bool dMat = true, bool dVerts = false, bool dInds = false)
{
	ImGui::Text("Offset: ");
	displayTransform(mesh._transform);

	ImGui::Separator();

	if (dMat)
	{
		ImGui::Text("Material: ");
		displayMaterial(mesh._baseMaterial);
		

		ImGui::Separator();
	}
	

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


	if (dInds)
	{
		ImGui::Text("Indices: ");
		std::stringstream result;
		std::copy(mesh._indices.begin(), mesh._indices.end(), std::ostream_iterator<int>(result, " "));
		ImGui::TextWrapped(result.str().c_str());
		ImGui::Separator();
	}
}



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
	if (ImGui::BeginChild("Actor"))	// yeet this out of function
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



static bool inTextStdStringHint(char* label, char* hint, std::string& inoutString)
{
	return (ImGui::InputTextWithHint(label, hint, inoutString.data(), inoutString.capacity() + 1,
		ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue,
		[](ImGuiInputTextCallbackData* data)->int
	{
		std::string* str = static_cast<std::string*>(data->UserData);
		IM_ASSERT(data->Buf == str->data());
		str->resize(data->BufTextLen);
		data->Buf = str->data();
		return 0;
	}, &inoutString));
}



static bool inTextStdString(char* label, std::string& inoutString)
{
	return (ImGui::InputText(label, inoutString.data(), inoutString.capacity() + 1,
		ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue,
		[](ImGuiInputTextCallbackData* data)->int
	{
		std::string* str = static_cast<std::string*>(data->UserData);
		IM_ASSERT(data->Buf == str->data());
		str->resize(data->BufTextLen);
		data->Buf = str->data();
		return 0;
	}, &inoutString));
}