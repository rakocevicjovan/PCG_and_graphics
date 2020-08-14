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
		if (ImGui::Begin("Material editor"))
		{
			for (UINT i = 0; i < _mat._texMetaData.size(); ++i)
			{
				TextureMetaData& tmd = _mat._texMetaData[i];
				Texture* tex = tmd._tex;

				if (tex != nullptr)
					GuiBlocks::displayTexture(tex);

				ImGui::Text(TEX_ROLE_NAMES[tmd._role]);

				for (UINT j = 0; j < 3; ++j)
				{
					ImGui::Text(TEX_MAPMODE_NAMES.at(tmd._mapMode[j]));
				}

				ImGui::Text("UV index: %d", tmd._uvIndex);

				// This should be a property of the underlying shader, not tmd as it is now
				ImGui::Text("Register: %d", tmd._regIndex);

				ImGui::Separator();
				ImGui::NewLine();
			}
		}

		ImGui::End();
	}
};