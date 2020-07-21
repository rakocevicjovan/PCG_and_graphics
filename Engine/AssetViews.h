#include "GuiBlocks.h"


class AssetViews
{
public:


	static void printSkeleton(Skeleton* skeleton)
	{
		if (ImGui::TreeNode("Skeleton"))
		{
			printBoneHierarchy(&skeleton->_bones[0]);	//_root
			ImGui::TreePop();
		}
	}



	static void printBoneHierarchy(Bone* bone)
	{
		if (!bone)
		{
			ImGui::TextColored(ImVec4(1., 0., 0., 1.), "Bone is nullptr");
			return;
		}

		
		if (ImGui::TreeNode(bone->_name.c_str()))
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();

				ImGui::Text("Local matrix");
				displayTransform(bone->_localMatrix);

				ImGui::Text("Inverse offset matrix");
				displayTransform(bone->_offsetMatrix);

				ImGui::EndTooltip();
			}

			for (Bone* cBone : bone->_children)
				printBoneHierarchy(cBone);
				

			ImGui::TreePop();
		}
	}
};