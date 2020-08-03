#pragma once
#include "FileUtilities.h"
#include <iostream>
#include <memory>



class AeonWriter
{
private:

	bool _active;

public:

	std::string _exportPath;



	AeonWriter() : _active(false), _exportPath("C:\\Users\\Senpai\\Desktop\\AeonTest\\") {}



	bool displayExportSettings()
	{
		bool result = false;

		if (ImGui::Begin("Export panel"))
		{
			// Settings
			GuiBlocks::inTextStdString("Export path", _exportPath);

			// Controls
			if (ImGui::Button("Close"))
				deactivate();

			ImGui::SameLine();

			if (ImGui::Button("Commit"))
			{
				result = std::filesystem::is_directory(_exportPath);
				/*if (FileUtils::fileExists(_exportPath))
					ImGui::OpenPopup("File already exists!");
				else
					result = true;*/
			}

			/*if(!result)
				result = displayOverwriteWarning();*/
		}
		ImGui::End();

		return result;
	}



	bool displayOverwriteWarning()
	{
		bool result = false;

		if (ImGui::BeginPopup("File already exists!"))
		{
			ImGui::Text("Are you sure you want to overwrite it?");

			if (ImGui::Button("Overwrite"))
			{
				ImGui::CloseCurrentPopup();
				result = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("No"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
		return result;
	}



	void writeAsset(SkeletalModel* _skModel, AssetLedger* _pLedger)
	{
		std::vector<UINT> meshIDs;
		for (int i = 0; i < _skModel->_meshes.size(); ++i)
		{
			uint32_t matId;
			{
				std::string matPath{ _exportPath + "//mat" + std::to_string(i) + ".aeon" };
				std::ofstream matOfs(matPath, std::ios::binary);
				cereal::BinaryOutputArchive matBoa(matOfs);
				_skModel->_meshes[i]._baseMaterial.serialize(matBoa, std::vector<UINT>{0u});
				matId = _pLedger->add(matPath, matPath, ResType::MATERIAL);
			}

			std::string meshPath{ _exportPath + "//mesh" + std::to_string(i) + ".aeon" };
			std::ofstream ofs(meshPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			_skModel->_meshes[i].serialize(boa, matId);
			meshIDs.push_back(_pLedger->add(meshPath, meshPath, ResType::SK_MESH));
		}

		std::vector<UINT> animIDs;
		for (UINT i = 0; i < _skModel->_anims.size(); ++i)
		{
			Animation& anim = _skModel->_anims[i];

			//std::string animName = anim.getName().length() == 0 ? "anim" + i : anim.getName();
			std::string animName = "anim" + std::to_string(i);
			std::string animPath{ _exportPath + "//" + animName + ".aeon" };
			std::ofstream ofs(animPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			anim.serialize(boa);
			animIDs.push_back(_pLedger->add(animPath, animPath, ResType::ANIMATION));
		}

		UINT skeletonID;
		{
			std::string skelPath{ _exportPath + "//skelly" + ".aeon" };
			std::ofstream ofs(skelPath, std::ios::binary);
			cereal::BinaryOutputArchive boa(ofs);
			//_skModel->_skeleton->save(boa);
			skeletonID = _pLedger->add(skelPath, skelPath, ResType::SKELETON);
		}

		std::ofstream ofs(_exportPath + "//skm.aeon", std::ios::binary);
		cereal::BinaryOutputArchive archie(ofs);
		_skModel->serialize(archie, meshIDs, animIDs, skeletonID);
	}



	inline void activate() { _active = true; }

	inline void deactivate() { _active = false; }

	inline bool isActive() { return _active; }
};