#include "GuiBlocks.h"
#include "SceneEditor.h"

// Contains helper functions to display skeleton hierarchy, model and mesh
class AssetViews
{
private:

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
				GuiBlocks::displayTransform(bone->_localMatrix);

				ImGui::Text("Inverse offset matrix");
				GuiBlocks::displayTransform(bone->_offsetMatrix);

				ImGui::EndTooltip();
			}

			for (Bone* cBone : bone->_children)
				printBoneHierarchy(cBone);

			ImGui::TreePop();
		}
	}


public:

	static void printSkeleton(Skeleton* skeleton)
	{
		if (ImGui::TreeNode("Skeleton"))
		{
			printBoneHierarchy(&skeleton->_bones[0]);	//_root
			ImGui::TreePop();
		}
	}


	static void printSkModel(SkeletalModel* skModel)
	{
		for (UINT i = 0; i < skModel->_meshes.size(); ++i)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode(&i, "Mesh %d", i))
			{
				SkeletalMesh* skm = &skModel->_meshes[i];
				GuiBlocks::displaySkMesh(skm);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}


	static void printModel(Model* model)
	{
		for (UINT i = 0; i < model->_meshes.size(); ++i)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode(&i, "Mesh %d", i))
			{
				Mesh* mesh = &model->_meshes[i];
				GuiBlocks::displayMesh(mesh);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
};