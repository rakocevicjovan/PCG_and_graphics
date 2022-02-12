#include "pch.h"

#include "DXCWrapper.h"

#include <dxc/dxcapi.h>     // Vulkan api include

// Not ideal because it depends on the path. Should add a build macro. 
// Without the full path path it breaks since there are two dxcompiler.lib versions - in windows dev kit and vulkan sdk
#pragma comment(lib, "C:/VulkanSDK/1.2.198.1/Lib/dxcompiler.lib")


using namespace Microsoft::WRL;


void DXCWrapper::init()
{
    if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler))))
    {
        __debugbreak();
    }

    if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils))))
    {
        __debugbreak();
    }

    if (FAILED(_utils->CreateDefaultIncludeHandler(&_includeHandler)))
    {
        __debugbreak();
    }
}


ComPtr<IDxcBlob> DXCWrapper::compile(const wchar_t* filename, ShaderType type)
{
    ComPtr<IDxcBlobEncoding> sourceBlob;

    if (FAILED(_utils->LoadFile(filename, nullptr, &sourceBlob)))
    {
        __debugbreak(); // File loading error
    }

    return compile(sourceBlob.Get(), type);
}


ComPtr<IDxcBlob> DXCWrapper::compile(IDxcBlobEncoding* sourceBlob, ShaderType type)
{
    DxcBuffer buffer{};
    buffer.Encoding = DXC_CP_ACP;
    buffer.Ptr = sourceBlob->GetBufferPointer();
    buffer.Size = sourceBlob->GetBufferSize();

    constexpr LPCWSTR profiles[]{ L"VS_6_0", L"GS_6_0", L"PS_6_0", L"TS_6_0", L"HS_6_0" };
    static_assert(std::size(profiles) == static_cast<size_t>(ShaderType::SHADER_TYPE_COUNT));

    LPCWSTR args[]
    {
        L"shader-error.txt",        // Filename for error reporting output
        L"-E", L"main",        // Entry function
        L"_T", type == ShaderType::VS ? profiles[0] : profiles[1],         // Target profile
        DXC_ARG_ALL_RESOURCES_BOUND,
#if _DEBUG
        DXC_ARG_DEBUG,
        DXC_ARG_SKIP_OPTIMIZATIONS,
#else
        DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
        DXC_ARG_WARNINGS_ARE_ERRORS
        // To strip debug info and reflections into a separate binary
        //L"-QStrip_reflect",
        //L"-Qstrip_debug",
    };

    ComPtr<IDxcOperationResult> opResults;
    HRESULT hr = _compiler->Compile(&buffer, args, std::size(args), _includeHandler.Get(), IID_PPV_ARGS(&opResults));

    if (SUCCEEDED(hr))
    {
        opResults->GetStatus(&hr);
    }

    if (FAILED(hr))
    {
        if (opResults)
        {
            ComPtr<IDxcBlobEncoding> errorsBlob;
            hr = opResults->GetErrorBuffer(&errorsBlob);
            if (SUCCEEDED(hr) && errorsBlob)
            {
                wprintf(L"Compilation failed with errors:\n%hs\n",
                    (const char*)errorsBlob->GetBufferPointer());
            }
        }
        // Handle compilation error...
    }

    ComPtr<IDxcBlob> compiled_shader_code;
    opResults->GetResult(&compiled_shader_code);

    return compiled_shader_code;
}