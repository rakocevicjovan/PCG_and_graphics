#pragma once

#include "VertSignature.h"

class Material;
struct ID3D11Device;

namespace ShaderPicker
{
	static void DisplayShaderPicker(VertSignature vertSig, Material* mat, ID3D11Device* device);
}
