#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class SwapChain
{
private:

	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
	bool _vSync{ false };

public:

	static DXGI_SWAP_CHAIN_DESC CreateDescription(
		UINT width, 
		UINT height,
		UINT numerator,
		UINT denominator,
		HWND windowHandle,
		bool fullScreen,
		bool vSync,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};

		// Number of buffers to cycle through with swap()
		swapChainDesc.BufferCount = 1;	// Double buffered, even if it's 1... oh well

		// Display mode options - if width or height are set to 0 they will be automatically obtained from the window handle
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = format;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = vSync ? numerator : 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = vSync ? denominator : 1;

		// Multi sampling options
		swapChainDesc.SampleDesc.Count = 1;		// Turn multisampling off.
		swapChainDesc.SampleDesc.Quality = 0;	// Maximum quality

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// Set the usage of the back buffer.
		swapChainDesc.OutputWindow = windowHandle;						// Set the handle for the window to render to.
		swapChainDesc.Windowed = !fullScreen;							// Set to full screen or windowed mode.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// Discard the back buffer contents after presenting.
		swapChainDesc.Flags = 0;										// Don't set the advanced flags.

		return swapChainDesc;
	}


	// @TODO should I even bother abstracting this?
	void init(ID3D11Device** device, DXGI_SWAP_CHAIN_DESC desc, D3D_FEATURE_LEVEL featureLevel, ID3D11DeviceContext** context)
	{
		_vSync = (desc.BufferDesc.RefreshRate.Numerator != 0);

		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			NULL,						// Adapter
			D3D_DRIVER_TYPE_HARDWARE,	// Hardware rendering, this won't really change, I cba...
			NULL,						//
			D3D11_CREATE_DEVICE_DEBUG, 
			&featureLevel, 
			1,
			D3D11_SDK_VERSION, 
			&desc, 
			_swapChain.GetAddressOf(), 
			device, 
			NULL, 
			context);

		if (FAILED(hr))
		{
			DebugBreak();
		}
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> getBackBufferPointer()
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBufferPtr;
		if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr)))
		{
			DebugBreak();
		}
		return backBufferPtr;
	}


	void present()
	{
		if (_vSync)
		{
			_swapChain->Present(1, 0);	// Lock to screen refresh rate.
		}
		else
		{
			_swapChain->Present(0, 0);	// Present as fast as possible.
		}
	}

	void swap()
	{

	}
};