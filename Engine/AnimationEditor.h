#pragma once
#include "Animation.h"
#include "GuiBlocks.h"


class AnimationEditor
{
	std::vector<Animation*> _animations;

public:

	void displayAnimation(Animation* anim)
	{
		ImGui::Text("Name: %s", anim->getName().c_str());

		UINT numChannels = anim->getNumChannels();

		ImGui::Text("Number of channels: %d", numChannels);

		ImGui::Text("Duration: %f; \nNum ticks: %f", anim->getDuration(), anim->getNumTicks());

		const std::map<std::string, AnimChannel>* channels = anim->getChannels();

		for(auto ac = channels->begin(); ac != channels->end(); ++ac)
		{
			const AnimChannel& ch = ac->second;

			UINT pkfc = ch._pKeys.size();
			UINT rkfc = ch._rKeys.size();
			UINT skfc = ch._sKeys.size();

			if (ImGui::TreeNode(ch._boneName.c_str()))
			{
				ImGui::Text("Nr. Pos KFs: %d", pkfc);
				ImGui::Text("Nr. Rot KFs: %d", rkfc);
				ImGui::Text("Nr. Scl KFs: %d", skfc);

				for (UINT i = 0; i < pkfc; ++i)
				{
					ImGui::Text("%d", i);
					/*
					ImGui::BeginTooltip();
					ImGui::Text("Tooltip!");
					ImGui::EndTooltip();
					*/

					// Absolutely not a way to do this, really lame... Need a sequencer...
					displayPositionKeyframe(ch._pKeys[i], i);

				}

				ImGui::TreePop();
			}

		}
	}



	static void displayPositionKeyframe(PosFrame pf, UINT i)
	{
		ImGui::PushID(i);
		ImGui::InputFloat3("##", &pf.pos.x, 3, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopID();
	}
};