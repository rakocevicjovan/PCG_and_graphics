#include "GuiBlocks.h"


class LoaderGUI
{
private:

	Model _curModel;

	std::string _assetPath;

	Mesh* _curMesh = nullptr;
	Texture* _curTex = nullptr;

public:

	void displayModel(ID3D11Device* device)
	{
		if (promptForPath(_assetPath))
		{
			std::filesystem::path realPath(_assetPath);	//C:\Users\Senpai\source\repos\PCG_and_graphics_stale_memes\Models\flintlock_rifle\flintlock.fbx

			if (FileUtils::fileExists(_assetPath))
			{
				std::filesystem::directory_entry file(realPath);
				_curModel.LoadModel(device, _assetPath);
			}	// else { pepehands(); }

			_curMesh = nullptr;
			_curTex = nullptr;
		}



		if (_curModel._meshes.size() > 0)
		{
			meshListWindow(_curModel._meshes);

			if (_curMesh)
			{
				textureListWindow(_curMesh->_textures);

				displaySelectedMesh();

				if(_curTex)
				{
					displaySelectedTexture();
				}
			}
		}

	}



private:

	bool promptForPath(std::string& path)
	{
		bool res = false;

		ImGui::SetNextWindowPos(ImVec2(0., 0.), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x, 256), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Meta data", NULL, ImGuiWindowFlags_NoMove))
		{
			//ImGui::Text("Path: ");	ImGui::SameLine();	ImGui::Text(path.c_str());

			res = inTextStdStringHint("Path", "Input absolute path to model (todo make rel to project)", path);
		}
		ImGui::End();

		return res;
	}



	void meshListWindow(std::vector<Mesh>& meshes)
	{
		//ImGui::SetNextWindowPos(ImVec2(0., 256.));
		//ImGui::SetNextWindowSize(ImVec2(512, 512));

		if (ImGui::Begin("Mesh list", NULL))
		{
			if (ImGui::ListBoxHeader("###Mesh list"))
			{
				for (int i = 0; i < meshes.size(); i++)
				{
					ImGui::PushID(i);
					if (ImGui::Selectable(std::string(std::to_string(i)).c_str()))
					{
						_curMesh = &meshes[i];
					}
					ImGui::PopID();
				}

				ImGui::ListBoxFooter();
			}
		}
		ImGui::End();
	}



	void textureListWindow(std::vector<Texture>& textures)
	{
		if (ImGui::Begin("Texture list", NULL))	//ImGuiWindowFlags_NoTitleBar
		{
			if (ImGui::ListBoxHeader("###Texture list"))
			{
				for (int i = 0; i < textures.size(); i++)
				{
					ImGui::PushID(i);
					if (ImGui::Selectable(std::string(std::to_string(i)).c_str()))
					{
						_curTex = &textures[i];
					}
					ImGui::PopID();
				}

				ImGui::ListBoxFooter();
			}
		}
		ImGui::End();
	}



	void displaySelectedMesh()
	{
		if (ImGui::Begin("Selected mesh"))
		{
			if (_curMesh)
				displayMesh(*_curMesh);
		}
		ImGui::End();
	}



	void displaySelectedTexture()
	{
		if (ImGui::Begin("Selected texture"))
		{
			if (_curTex)
			{
				displayTexture(_curTex->_srv, _curTex->getName(),
					_curTex->getW(),
					_curTex->getH(),
					_curTex->getN());
			}
		}
		ImGui::End();
	}
};