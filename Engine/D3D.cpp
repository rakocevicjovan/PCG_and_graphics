#include "pch.h"
#include "D3D.h"



bool D3D::initialize(int windowWidth, int windowHeight, bool vsync, HWND hwnd, bool fullscreen)
{
	// Store the vsync setting.
	_VSyncEnabled = vsync;

	// Create a DirectX graphics interface factory.
	Microsoft::WRL::ComPtr<IDXGIFactory> factory;
	if(FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(factory.GetAddressOf()))))
		return false;

	// Use the factory to create an adapter for the primary graphics interface (video card).
	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	if(FAILED(factory->EnumAdapters(0, adapter.GetAddressOf())))
		return false;

	// Enumerate the primary adapter output (monitor).
	Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
	if(FAILED(adapter->EnumOutputs(0, adapterOutput.GetAddressOf())))
		return false;

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)
	UINT numModes{};
	if(FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL)))
		return false;

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	std::vector<DXGI_MODE_DESC> displayModeList(numModes);

	// Now fill the display mode list structures.
	if(FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.data())))
		return false;

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	uint32_t numerator{ 0u };
	uint32_t denominator{ 0u };

	for(uint32_t i = 0; i < numModes; i++)
	{
		if(displayModeList[i].Width == (unsigned int)windowWidth)
		{
			if(displayModeList[i].Height == (unsigned int)windowHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	DXGI_ADAPTER_DESC adapterDesc;
	if (FAILED(adapter->GetDesc(&adapterDesc)))
	{
		OutputDebugStringA("Failed to get video card description.");
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	size_t stringLength;
	int error = wcstombs_s(&stringLength, _videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		OutputDebugStringA("Failed to get video card description.");
		return false;
	}

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

	// Create the swap chain, Direct3D device, and Direct3D device context. D3D11_CREATE_DEVICE_DEBUG useful for now, not in release
	
	{
		Microsoft::WRL::ComPtr<ID3D11Device> deviceBase;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> contextBase;

		auto swapChainDesc = SwapChain::CreateDescription(windowWidth, windowHeight, numerator, denominator, hwnd, fullscreen, vsync, DXGI_FORMAT_R8G8B8A8_UNORM);
		_swapChain.init(swapChainDesc, featureLevel, vsync, &deviceBase, &contextBase);

		deviceBase->QueryInterface<ID3D11Device5>(_device.GetAddressOf());
		contextBase->QueryInterface<ID3D11DeviceContext4>(_deviceContext.GetAddressOf());
	}

	// Get the pointer to the back buffer.
	auto backBufferPtr = _swapChain.getBackBufferPointer();
	
	// Create the render target view with the back buffer pointer. Uknown format means adjust to parent.
	_renderTarget.fromExistingTexture(_device.Get(), backBufferPtr.Get(), windowWidth, windowHeight);

	// Depth stencil states - these are just hardcoded right now as I don't need anything special.
	DepthStencilDesc depthStencilDesc{};
	depthStencilDesc.depthFunc = DepthStencilDesc::COMP::LESS_EQUAL;
	_depthStencilLessEquals.createDepthStencilState(_device.Get(), depthStencilDesc);

	depthStencilDesc.depthFunc = DepthStencilDesc::COMP::LESS;
	_depthStencilLess.createDepthStencilState(_device.Get(), depthStencilDesc);

	depthStencilDesc.depthTest = false;
	depthStencilDesc.stencilTest = false;
	depthStencilDesc.depthFunc = DepthStencilDesc::COMP::ALWAYS;
	_depthStencilNoDepthTest.createDepthStencilState(_device.Get(), depthStencilDesc);

	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc{};

	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;	// Special case for drawing lines if multisampling is disabled already.

	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = false;

	rasterDesc.DepthClipEnable = true;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	
	rasterDesc.FillMode = D3D11_FILL_SOLID;

	rasterDesc.ScissorEnable = false;		// Enabling this can be useful but requires a scissor rect to draw.

	// Create the rasterizer state from the description we just filled out.
	if (FAILED(_device->CreateRasterizerState(&rasterDesc, &_r_s_solid_cull)))
		return false;

	rasterDesc.CullMode = D3D11_CULL_NONE;
	if (FAILED(_device->CreateRasterizerState(&rasterDesc, &_r_s_solid_no_cull)))
		return false;

	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	if (FAILED(_device->CreateRasterizerState(&rasterDesc, &_r_s_wireframe)))
		return false;

	// Blending code @TODO see why it messes with texture.... turn off until then
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (FAILED(_device->CreateBlendState(&blendDesc, &_blendState)))
		return false;

	// Modify the description to create an alpha disabled blend state description.
	D3D11_BLEND_DESC noBlendDesc{};
	noBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	noBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	// Create the blend state using the description.
	if (FAILED(_device->CreateBlendState(&noBlendDesc, &_noBlendState)))
		return false;

	
	_viewport._viewport = Viewport::CreateViewport((float)windowWidth, (float)windowHeight);

	
	// Set the defaults
	// 
	// Set the depth stencil state. Either of these works well enough as the default one.
	//_depthStencilLess.bind(_deviceContext.Get());
	_depthStencilLessEquals.bind(_deviceContext.Get());

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	_renderTarget.bind(_deviceContext.Get());

	_deviceContext->RSSetState(_r_s_solid_cull.Get());

	TurnOffAlphaBlending();

	_deviceContext->RSSetViewports(1, &_viewport._viewport);

    return true;
}


void D3D::ClearColourDepthBuffers()
{
	_renderTarget.clear(_deviceContext.Get());
}


void D3D::present()
{
	_swapChain.present();
}


void D3D::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, _videoCardDescription);
	memory = _videoCardMemory;
}


void D3D::SetBackBufferRenderTarget()
{
	_deviceContext->RSSetViewports(1, &_viewport._viewport);
	_renderTarget.bind(_deviceContext.Get());
	_renderTarget.clear(_deviceContext.Get());
}


void D3D::D3D::TurnOnAlphaBlending()
{
	float blendFactor[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

	_deviceContext->OMSetBlendState(_blendState, nullptr, 0xffffffff);
}


void D3D::D3D::TurnOffAlphaBlending()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	_deviceContext->OMSetBlendState(_noBlendState, blendFactor, 0xffffffff);
}


void D3D::setRSSolidCull()
{
	_deviceContext->RSSetState(_r_s_solid_cull.Get());
}


void D3D::setRSSolidNoCull()
{
	_deviceContext->RSSetState(_r_s_solid_no_cull.Get());
}


void D3D::setRSWireframe()
{
	_deviceContext->RSSetState(_r_s_wireframe.Get());
}

void D3D::setDSSNoTest()
{
	_depthStencilNoDepthTest.bind(_deviceContext.Get());
}


void D3D::setDSSLessEquals()
{
	_depthStencilLessEquals.bind(_deviceContext.Get());
}


void D3D::setDSSLess()
{
	_depthStencilLess.bind(_deviceContext.Get());
}