#include "GuiBlocks.h"
#include "SceneEditor.h"
#include <span>

// Contains helper functions to display skeleton hierarchy, model and mesh
class AssetViews
{
private:
	
	static void PrintBoneHierarchy(std::vector<Bone>& bones, Bone& bone)
	{
		auto [offset, size] = bone._children;

		if (ImGui::TreeNode(bone._name.c_str()))
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();

				ImGui::Text("Local matrix");
				GuiBlocks::displayTransform(bone._localMatrix);

				ImGui::Text("Inverse offset matrix");
				GuiBlocks::displayTransform(bone._invBindPose);

				ImGui::EndTooltip();
			}

			for (auto j = offset; j < offset + size; ++j)
			{
				PrintBoneHierarchy(bones, bones[j]);
			}

			ImGui::TreePop();
		}
	}


	static void PrintBoneHierarchy(std::vector<Bone>& bones, uint16_t boneIndex)
	{
		if (bones.size() == 0)
		{
			ImGui::Text("Skeleton invalid, no bones found.");
			return;
		}

		for (uint32_t i = 0; i < bones.size(); ++i)
		{
			Bone& curBone = bones[i];

			if (ImGui::TreeNode(curBone._name.c_str()))
			{
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();

					ImGui::Text("Local matrix");
					GuiBlocks::displayTransform(curBone._localMatrix);

					ImGui::Text("Inverse offset matrix");
					GuiBlocks::displayTransform(curBone._invBindPose);

					ImGui::EndTooltip();
				}
				ImGui::TreePop();
			}
		}
	}


public:

	static void printSkeleton(Skeleton* skeleton)
	{
		if (ImGui::TreeNode("Skeleton"))
		{
			PrintBoneHierarchy(skeleton->_bones, skeleton->_bones[0]);	//_root
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