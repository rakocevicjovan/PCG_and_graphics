#pragma once

#include <wrl/client.h>
#include "Shader.h"

struct IDxcCompiler3;
struct IDxcUtils;
struct IDxcIncludeHandler;
struct IDxcBlobEncoding;
struct IDxcBlob;

class DXCWrapper
{
private:
	Microsoft::WRL::ComPtr<IDxcCompiler3> _compiler;
	Microsoft::WRL::ComPtr<IDxcUtils> _utils;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> _includeHandler;

public:
	void init();
	Microsoft::WRL::ComPtr<IDxcBlob> compile(const wchar_t* filename, ShaderType type);
	Microsoft::WRL::ComPtr<IDxcBlob> compile(IDxcBlobEncoding* source_blob, ShaderType type);
};