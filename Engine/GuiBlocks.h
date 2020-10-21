#pragma once
#include "Mesh.h"
#include "SkeletalMesh.h"
#include "GameObject.h"
#include "IMGUI/imgui.h"
#include <d3d11_4.h>
#include <string>
#include <sstream>
#include <algorithm>



namespace GuiBlocks
{

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



	static void displayTexture(const Texture* t)
	{
		ImGui::Text(t->getName().c_str());

		ImGui::Text("Width: %i \nHeight: %i \nChannels: %i", t->w(), t->h(), t->nc());

		if (t->_srv)
			ImGui::Image(t->_srv, ImVec2(256., 256.));
	}



	static void displayShaders(VertexShader* vs, PixelShader* ps)
	{
		ImGui::Text("Shaders: ");

		ImGui::Indent();

		if (vs)
		{
			std::string vsName(vs->_path.begin(), vs->_path.end());
			ImGui::Text(vsName.c_str());
		}
		else
			ImGui::Text("None loaded");

		if (ps)
		{
			std::string psName(ps->_path.begin(), ps->_path.end());
			ImGui::Text(psName.c_str());
		}
		else
			ImGui::Text("None loaded");
	}



	static void displayMaterial(Material* mat)
	{
		ImGui::BeginGroup();

		displayShaders(mat->getVS(), mat->getPS());

		ImGui::Unindent();

		ImGui::Text("Textures");
		ImGui::Indent();

		for (int i = 0; i < mat->_texMetaData.size(); i++)
		{
			ImGui::Text(TEX_ROLE_NAMES.at(mat->_texMetaData[i]._role));
			ImGui::SameLine();
			if (mat->_texMetaData[i]._tex)
			{
				ImGui::Text(mat->_texMetaData[i]._tex->getName().c_str());
				ImGui::Image(mat->_texMetaData[i]._tex->_srv, ImVec2(300, 300));
			}
			else
			{
				ImGui::NewLine();
			}
		}

		ImGui::Unindent();

		ImGui::Checkbox("Opaque: ", &mat->_opaque);

		ImGui::EndGroup();
	}



	static void displayMesh(Mesh* mesh, bool dMat = true)
	{
		ImGui::Text("Offset matrix: ");
		displayTransform(mesh->_transform);

		ImGui::Separator();

		if (dMat)
		{
			ImGui::Text("Material: ");
			displayMaterial(mesh->_material.get());
			ImGui::Separator();
		}
	}



	static void displaySkMesh(SkeletalMesh* mesh)
	{
		ImGui::Text("Transform: ");
		ImGui::Indent();
		displayTransform(mesh->_transform);
		ImGui::Unindent();
		ImGui::NewLine();

		ImGui::Text("Stats: ");
		ImGui::Indent();
		ImGui::Text("Vertex count: %d", mesh->_vertices.size());
		ImGui::Text("Index count:  %d", mesh->_indices.size());
		ImGui::Unindent();
		ImGui::NewLine();

		ImGui::Text("Material: ");
		ImGui::Indent();
		displayMaterial(mesh->getMaterial());
		ImGui::Unindent();
		ImGui::NewLine();
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
		displayMesh(r.mesh);
		ImGui::PopID();

		ImGui::Text("Material");
		displayMaterial(r.mat);

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



	static bool inputStdString(char* label, std::string& inoutString)
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



	static bool displayOverwriteWarning(const char* filePath)
	{
		bool result;

		if (ImGui::BeginPopup("File already exists!"))
		{
			ImGui::Text("File path: \"%s\"", filePath);
			ImGui::Text("Are you sure you want to overwrite it?");

			if (ImGui::Button("Overwrite"))
			{
				ImGui::CloseCurrentPopup();
				result = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("No"))
			{
				ImGui::CloseCurrentPopup();
				result = false;
			}
			ImGui::EndPopup();
		}
		return result;
	}



	// Useful for outputting debug textures
	static void displayImage(ID3D11ShaderResourceView* srv, float pX, float pY, float sX, float sY, const char* title)
	{
		ImGui::SetNextWindowPos(ImVec2(pX, pY), ImGuiCond_Once);

		ImGui::Begin(title, false);
		ImGui::Image(srv, ImVec2(sX, sY));
		ImGui::End();
	}



	// Doesn't do anything, syntax refresher (imagine having documentation)
	static bool contextMenu()
	{
		if (ImGui::BeginPopupContextItem("Bruh"))	// ImGui::BeginPopupContextWindow();
		{
			if (ImGui::Selectable("zone"))
			{
			}
		}
	}
};