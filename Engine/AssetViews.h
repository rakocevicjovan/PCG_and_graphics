#include "GuiBlocks.h"


class AssetViews
{
public:


	static void printSkeleton(Skeleton* skeleton)
	{
		if (ImGui::TreeNode("Skeleton"))
		{
			printBoneHierarchy(skeleton->_root);
			ImGui::TreePop();
		}
	}



	static void printBoneHierarchy(Bone* bone)
	{
		if (!bone)
		{
			ImGui::Text("Bone is nullptr");
			return;
		}

		if (ImGui::TreeNode("N: %s ; Idx: %d", bone->_name.c_str(), bone->_index))
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(ImVec4(1., 0., 0., 1.), "Local matrix");
				displayTransform(bone->_localMatrix);
				ImGui::TextColored(ImVec4(0., 0., 1., 1.), "Inverse offset matrix");
				displayTransform(bone->_offsetMatrix);
				ImGui::EndTooltip();
			}

			for (Bone* cBone : bone->_children)
				printBoneHierarchy(cBone);

			ImGui::TreePop();
		}
	}
};