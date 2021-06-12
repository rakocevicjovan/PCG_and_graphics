#include "GuiBlocks.h"
#include "SceneEditor.h"
#include "Bone.h"
#include <span>

// Contains helper functions to display skeleton hierarchy, model and mesh
class AssetViews
{
private:
	
	static void PrintBoneHierarchy(std::vector<Bone>& bones, Bone& bone)
	{
		auto [offset, size] = bone._children;

		if (bone.isInfluenceBone())
		{
			ImGui::PushStyleColor(0, ImVec4(.2, 7., 1., 1.));
		}

		if (ImGui::TreeNode(bone.name()))
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

		if (bone.isInfluenceBone())
		{
			ImGui::PopStyleColor();
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

			if (ImGui::TreeNode(curBone.name()))
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


	static void printSkModel(SkModel* skModel)
	{
		if (ImGui::TreeNode("Mesh node hierarchy: "))
		{
			for (auto& meshNode : skModel->_meshNodeTree)
			{
				//GuiBlocks::displayTransform(meshNode.transform);

				ImGui::Text("Contained mesh indices: ");
				ImGui::Indent();
				for (auto& meshIndex : meshNode.meshes)
				{
					ImGui::Text("%i", meshIndex);
				}
				ImGui::Unindent();

				ImGui::Text("Parent node: %d", meshNode.parent);
			}
			ImGui::TreePop();
		}

		for (UINT i = 0; i < skModel->_meshes.size(); ++i)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode(&i, "Mesh %d", i))
			{
				Mesh* skm = &skModel->_meshes[i];
				GuiBlocks::displayMesh(skm);
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