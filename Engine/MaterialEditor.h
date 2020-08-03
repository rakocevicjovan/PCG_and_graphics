#pragma once
#include "Material.h"
#include "GuiBlocks.h"

class MaterialEditor
{
	Material& _mat;

public:

	MaterialEditor(Material& mat) : _mat(mat) {}



	void display()
	{
		if (!ImGui::Begin("Material editor"))
			return;

		for (UINT i = 0; i < _mat._texMetaData.size(); ++i)
		{
			TextureMetaData& tmd = _mat._texMetaData[i];
			Texture& tex = *tmd._tex;

			GuiBlocks::displayTexture(tex);
			/*ImGui::Text(tex.getName().c_str());
			ImGui::Text("Width: %i \nHeight: %i \nChannels: %i", tex.getW(), tex.getH(), tex.getN());
			if (tex._srv != nullptr)
				ImGui::Image(tex._srv, ImVec2(512., 512.));*/

			ImGui::Text(TEX_ROLE_NAMES[tmd._role]);

			for (UINT j = 0; j < 3; ++j)
			{
				ImGui::Text(TEX_MAPMODE_NAMES.at(tmd._mapMode[j]));
			}

			ImGui::Text("UV index: %s", std::to_string(tmd._uvIndex).c_str());

			ImGui::Text("Register: %s", std::to_string(tmd._regIndex).c_str());

			ImGui::Separator();
			ImGui::NewLine();
		}

		ImGui::End();
	}

};