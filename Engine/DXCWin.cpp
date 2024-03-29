#include "pch.h"

#include "DXCWin.h"
#include <dxcapi.h> // This is the include from the windows kit.

using namespace Microsoft::WRL;


void DXC::init()
{
    if (FAILED(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(_library.GetAddressOf()))))
    {
        __debugbreak();
    }

    if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(_compiler.GetAddressOf()))))
    {
        __debugbreak();
    }
}


ComPtr<IDxcBlob> DXC::compile(const wchar_t* filename)
{
    uint32_t codePage = CP_UTF8;
    ComPtr<IDxcBlobEncoding> sourceBlob;

    HRESULT hr = _library->CreateBlobFromFile(filename, &codePage, &sourceBlob);

    if (FAILED(hr))
    {
        __debugbreak(); // File loading error
    }

    ComPtr<IDxcOperationResult> opResult;
    hr = _compiler->Compile(
        sourceBlob.Get(),   // pSource
        filename,           // pSourceName
        L"main",            // pEntryPoint
        L"ps_6_0",          // pTargetProfile, ps/vs/cs etc. might have to be upper or lower case not sure now
        NULL, 0,            // pArguments, argCount
        NULL, 0,            // pDefines, defineCount
        NULL,               // pIncludeHandler
        &opResult);         // ppResult

    if (SUCCEEDED(hr))
    {
        opResult->GetStatus(&hr);
    }

    if (FAILED(hr))
    {
        if (opResult)
        {
            ComPtr<IDxcBlobEncoding> errorsBlob;
            hr = opResult->GetErrorBuffer(&errorsBlob);
            if (SUCCEEDED(hr) && errorsBlob)
            {
                wprintf(L"Compilation failed with errors:\n%hs\n",
                    (const char*)errorsBlob->GetBufferPointer());
            }
        }
        // Handle compilation error...
    }

    ComPtr<IDxcBlob> code;
    opResult->GetResult(&code);

    return code;
}