#pragma once


#include <wrl/client.h>
// Impl hlsl->SPIRV for vulkan

struct IDxcLibrary;
struct IDxcCompiler;
struct IDxcBlob;

class DXC
{
private:
	Microsoft::WRL::ComPtr<IDxcLibrary> _library;
	Microsoft::WRL::ComPtr<IDxcCompiler> _compiler;

public:
	void init();
	Microsoft::WRL::ComPtr<IDxcBlob> compile(const wchar_t* filename);
};