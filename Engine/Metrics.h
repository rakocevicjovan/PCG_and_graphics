#pragma once
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <cstdint>
#include <vector>
#include <cassert>

class Metrics
{
private:

    template<typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   std::vector<ComPtr<IDXGIAdapter3>> _adapters;

    void enumerateAdapters()
    {
        ComPtr<IDXGIFactory4> dxgiFactory{};
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
        {
            assert(false && "DXGI Factory 4 could not be initialized.");
        }

        UINT adapterIndex{ 0u };
        ComPtr<IDXGIAdapter1> tmpDxgiAdapter{};

        while (dxgiFactory->EnumAdapters1(adapterIndex, &tmpDxgiAdapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC1 desc;
           
            if (!FAILED(tmpDxgiAdapter->GetDesc1(&desc)))
            {
                _adapters.push_back({});

                if (desc.Flags == 0)    // Prunes "software renderer", I guess it's the integrated gpu?
                {
                    if (FAILED(tmpDxgiAdapter->QueryInterface(IID_PPV_ARGS(&_adapters.back()))))
                    {
                        _adapters.pop_back();
                    }
                }
            }

            tmpDxgiAdapter.Reset();
            ++adapterIndex;
        }
    }


public:

	std::vector<DXGI_QUERY_VIDEO_MEMORY_INFO> getMemoryInfo()
	{
        std::vector<DXGI_QUERY_VIDEO_MEMORY_INFO> result(_adapters.size());

        for (int i = 0; i < _adapters.size(); ++i)
        {
           _adapters[i]->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &result[i]);
        }

        return result;
	}

};