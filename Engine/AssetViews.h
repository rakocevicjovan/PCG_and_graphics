#include "GuiBlocks.h"
#include "SceneEditor.h"

// Contains helper functions to display skeleton hierarchy, model and mesh
class AssetViews
{
private:

	static void printBoneHierarchy(std::vector<Bone>& bones, uint16_t boneIndex)
	{
		if (boneIndex == (~0))
		{
			ImGui::TextColored(ImVec4(1., 0., 0., 1.), "Bone missing.");
			return;
		}

		Bone& currentBone = bones[boneIndex];

		if (ImGui::TreeNode(currentBone._name.c_str()))
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();

				ImGui::Text("Local matrix");
				GuiBlocks::displayTransform(currentBone._localMatrix);

				ImGui::Text("Inverse offset matrix");
				GuiBlocks::displayTransform(currentBone._offsetMatrix);

				ImGui::EndTooltip();
			}

			for (uint16_t boneIndex : currentBone._children)
			{
				printBoneHierarchy(bones, currentBone._index);
			}
				

			ImGui::TreePop();
		}
	}


public:

	static void printSkeleton(Skeleton* skeleton)
	{
		if (ImGui::TreeNode("Skeleton"))
		{
			printBoneHierarchy(skeleton->_bones, 0u);	//_root
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