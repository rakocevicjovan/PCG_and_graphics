#include "pch.h"

#include "ShaderPicker.h"


namespace ShaderPicker
{

	// Good idea but rethink impl. and avoid the clutter here
void DisplayShaderPicker(VertSignature vertSig, Material* mat, ID3D11Device* device)
{
	//// For UI
	//struct LightModelIndex
	//{
	//	std::string name;
	//	SHG_LIGHT_MODEL index;
	//};
	//uint64_t shaderKey{ 0 };

	//static const std::vector<LightModelIndex> lmiOptions
	//{
	//	{ "NONE",		SHG_LM_NONE},
	//	{ "LAMBERT",	SHG_LM_LAMBERT},
	//	{ "PHONG",		SHG_LM_PHONG}
	//};

	//// Default lambert
	//static const LightModelIndex* selected = &lmiOptions[1];

	//if (ImGui::Begin("Shader picker"))
	//{
	//	if (ImGui::BeginCombo("Light model", selected->name.data()))
	//	{
	//		for (UINT i = 0; i < lmiOptions.size(); ++i)
	//		{
	//			if (ImGui::Selectable(lmiOptions[i].name.c_str()))
	//				selected = &lmiOptions[i];
	//		}
	//		ImGui::EndCombo();
	//	}
	//}

	//shaderKey = ShaderGenerator::CreateShaderKey(vertSig, mat, selected->index);

	//ImGui::Text("Key: %lu", shaderKey);

	//if (ImGui::Button("Create shader"))
	//{
	//	CreateShader(device, shaderKey, vertSig, mat);
	//}

	//ImGui::End();
}

}