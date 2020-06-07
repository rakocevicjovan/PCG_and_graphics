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

		ImGui::Text("Duration: %f; \nNum ticks: %d", anim->getDuration(), anim->getNumTicks());

		const std::map<std::string, AnimChannel>* channels = anim->getChannels();

		for(auto ac = channels->begin(); ac != channels->end(); ++ac)
		{
			const AnimChannel& ch = ac->second;

			UINT pkfc = ch.pKeys.size();
			UINT rkfc = ch.rKeys.size();
			UINT skfc = ch.sKeys.size();

			if (ImGui::TreeNode(ch.jointName.c_str()))
			{
				ImGui::Text("Nr. Pos KFs: %d", pkfc);
				ImGui::Text("Nr. Rot KFs: %d", rkfc);
				ImGui::Text("Nr. Scl KFs: %d", skfc);

				for (UINT i = 0; i < pkfc; ++i)
				{
					ImGui::Button((const char*)(i));
					ImGui::BeginTooltip();
					ImGui::EndTooltip();
				}

				ImGui::TreePop();
			}

		}
	}
};