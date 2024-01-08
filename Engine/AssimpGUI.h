#pragma once
#include "GuiBlocks.h"

class AssimpGUI
{
public:

	static bool printaiNode(aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform)
	{
		std::string nodeName("Node: ");
		nodeName += node->mName.C_Str();

		if (ImGui::TreeNode(nodeName.c_str()))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.6f, 0.f, 1.f, 1.f));

			aiMatrix4x4 concatenatedTransform = node->mTransformation * parentTransform;

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();

				ImGui::TextColored(ImVec4(1., 0., 0., 1.), "Local matrix");

				SMatrix concatSMat = AssimpWrapper::aiMatToSMat(concatenatedTransform);
				GuiBlocks::displayTransform(concatSMat);

				SQuat squat = SQuat::CreateFromRotationMatrix(concatSMat);
				ImGui::InputFloat4("Quat: ", &squat.x, "%.3f", ImGuiInputTextFlags_ReadOnly);

				ImGui::EndTooltip();
			}


			ImGui::Text("Mesh count: %d", node->mNumMeshes);

			if (node->mNumMeshes > 0)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0., 1., 0., 1.));
				if (ImGui::TreeNode("Meshes"))
				{
					for (unsigned int i = 0; i < node->mNumMeshes; i++)
					{
						aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

						ImGui::PushID(i);
						if (ImGui::TreeNode("Mesh name and node index: %s (%d)", mesh->mName.data, i))
						{
							printAiMesh(scene, mesh, concatenatedTransform);
							ImGui::TreePop();
						}
						ImGui::PopID();

					}
					ImGui::TreePop();
				}
				ImGui::PopStyleColor();
			}


			ImGui::Separator();

			ImGui::Text("Child count: %d", node->mNumChildren);

			if (node->mNumChildren > 0)
			{
				if (ImGui::TreeNode("Children"))
				{
					for (unsigned int i = 0; i < node->mNumChildren; i++)
					{
						ImGui::PushID(i);
						printaiNode(node->mChildren[i], scene, concatenatedTransform);
						ImGui::Separator();
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
			}

			ImGui::PopStyleColor();
			ImGui::TreePop();
		}

		return true;
	}



	static void printAiMesh(const aiScene* scene, aiMesh* mesh, aiMatrix4x4 parentGlobal)
	{
		// Just because they are there doesn't mean they are useful, sadly... 
		// It's infuriating how many issues need to be ironed out during import, like removing duplicates
		UINT numUVChannels = mesh->GetNumUVChannels();
		UINT* numUVComponents = mesh->mNumUVComponents;

		ImGui::Text("Nr. of UV channels: %d", numUVChannels);
		ImGui::Text("Nr. of UV components per channel: ");

		ImGui::Indent();
		for (auto i = 0u; i < numUVChannels; i++)
			ImGui::Text("%d: %d ", i, numUVComponents[i]);
		ImGui::Unindent();

		ImGui::NewLine();

		ImGui::Text("Vertex count: %d", mesh->mNumVertices);
		ImGui::Text("Index count: %d", mesh->mNumFaces * 3);	// I always use assimp's triangulate flag
		ImGui::Text("Face count: %d", mesh->mNumFaces);
		ImGui::Text("Has (bi)tangents: %d", mesh->HasTangentsAndBitangents());

		ImGui::NewLine();

		printAiMaterial(scene, mesh);
	}



	static void printAiMaterial(const aiScene* aiScene, aiMesh* mesh)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.4f, .5f, .7f, 1.f));
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* aiMat = aiScene->mMaterials[mesh->mMaterialIndex];

			ImGui::Text("Material: %s", aiMat->GetName().C_Str());

			// Diffuse maps
			printMaterialTextures(aiScene, aiMat, aiTextureType_DIFFUSE, "texture_diffuse", DIFFUSE);

			// Normal maps
			printMaterialTextures(aiScene, aiMat, aiTextureType_NORMALS, "texture_normal", NORMAL);

			// Specular maps
			printMaterialTextures(aiScene, aiMat, aiTextureType_SPECULAR, "texture_specular", SPECULAR);

			// Shininess maps
			printMaterialTextures(aiScene, aiMat, aiTextureType_SHININESS, "texture_shininess", SHININESS);

			// Opacity maps
			printMaterialTextures(aiScene, aiMat, aiTextureType_OPACITY, "texture_opacity", OPACITY);

			// Displacement maps
			printMaterialTextures(aiScene, aiMat, aiTextureType_DISPLACEMENT, "texture_disp", DPCM);

			// Ambient occlusion maps
			printMaterialTextures(aiScene, aiMat, aiTextureType_AMBIENT, "texture_AO", AMB_OCCLUSION);

			// Other maps
			printMaterialTextures(aiScene, aiMat, aiTextureType_UNKNOWN, "texture_other", OTHER);

			// Weird properties... that I never really saw trigger
			printMaterialTextures(aiScene, aiMat, aiTextureType_NONE, "texture_property", OTHER);

			ImGui::Unindent();
		}
		else
		{
			ImGui::Text("Material: none");
		}

		ImGui::PopStyleColor();
	}



	static void printMaterialTextures(const aiScene* aiScene, aiMaterial *mat, aiTextureType type, std::string typeName, TextureRole role)
	{
		UINT numThisType = mat->GetTextureCount(type);
		typeName += " count: " + std::to_string(numThisType);

		ImGui::Text(typeName.c_str());

		ImGui::Indent();

		//iterate all textures of relevant related to the material
		for (UINT i = 0; i < numThisType; ++i)
		{
			// Try to load this texture from file
			aiString texPath;
			mat->GetTexture(type, i, &texPath);

			// This assumes files are exported with relative paths... which seems to be a big if...
			//std::string modelFolderPath = _path.substr(0, _path.find_last_of("/\\")) + "\\";
			//std::string modelFolderPath = std::filesystem::path(_path).parent_path().string();
			//std::string texPath = modelFolderPath + std::string(obtainedTexturePath.data);
			//std::string texName = std::filesystem::path(std::string(obtainedTexturePath.C_Str())).filename().string();


			bool texFound = FileUtils::fileExists(texPath.C_Str());

			if (texFound)
			{
				ImGui::Text("Path: %s", texPath.C_Str());
			}
			else
			{
				const aiTexture* aiTex = nullptr;

				if (aiScene->mTextures)
					aiTex = aiScene->GetEmbeddedTexture(texPath.C_Str());

				if (aiTex)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0., 1., 1., 1.));
					ImGui::Text("This texture is embedded!");
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::Text("Path: %s", texPath.C_Str());
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1., 0., 0., 1.));
					ImGui::Text(" ( WARNING: NOT FOUND! )");
					ImGui::PopStyleColor();
				}

			}
		}

		ImGui::Unindent();
	}



	static void printSceneAnimations(const aiScene* aiScene)
	{
		if (!aiScene->HasAnimations())
			return;

		if (ImGui::TreeNode("Animation list"))
		{
			for (auto i = 0u; i < aiScene->mNumAnimations; ++i)
				printAiAnimation(aiScene->mAnimations[i]);

			ImGui::TreePop();
		}
	}



	static void printAiAnimation(aiAnimation* sceneAnimation)
	{
		int numChannels = sceneAnimation->mNumChannels;

		if (ImGui::TreeNode(sceneAnimation->mName.C_Str()))
		{
			ImGui::Text("Num channels: %d", numChannels);

			for (int j = 0; j < numChannels; ++j)
			{
				aiNodeAnim* channel = sceneAnimation->mChannels[j];

				if (ImGui::TreeNode(channel->mNodeName.C_Str()))
				{
					printAiAnimationTrack(channel);
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}



	static void printAiAnimationTrack(aiNodeAnim* channel)
	{
		ImGui::Text("Num scaling keys: %d", channel->mNumScalingKeys);
		ImGui::Text("Num rotation keys: %d", channel->mNumRotationKeys);
		ImGui::Text("Num position keys: %d", channel->mNumPositionKeys);
	}



	static void printAiSceneTextures(const aiScene* aiScene, std::vector<aiString> extTexPaths)
	{
		if (aiScene->mTextures)
		{
			if (ImGui::TreeNode("Embedded"))
			{
				for (auto i = 0u; i < aiScene->mNumTextures; ++i)
				{
					const aiTexture* tex = aiScene->mTextures[i];
					ImGui::Text("Name: %s", tex->mFilename.C_Str());
					ImGui::Text("Width: %d; Height: %d;", tex->mWidth, tex->mHeight);
					ImGui::Text("Format hint: %s", tex->achFormatHint);
				}

				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::Text("No embedded textures found");
		}

		if (extTexPaths.size() > 0)
		{
			if (ImGui::TreeNode("External"))
			{
				for (int i = 0; i < extTexPaths.size(); ++i)
					ImGui::Text(extTexPaths[i].C_Str());

				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::Text("No external textures found");
		}
	}
};