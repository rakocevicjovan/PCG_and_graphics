#include "D3D.h"



D3D::D3D() :
_swapChain(0),
_device(0),
_deviceContext(0),
_r_s_solid_cull(0)
{
}



D3D::~D3D()
{ 
	Shutdown();
}



bool D3D::Initialize(int windowWidth, int windowHeight, bool vsync, HWND hwnd, bool fullscreen)
{
	HRESULT result;

	// Store the vsync setting.
	_VSyncEnabled = vsync;

	// Create a DirectX graphics interface factory.
	IDXGIFactory* factory;
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if(FAILED(result))
		return false;

	// Use the factory to create an adapter for the primary graphics interface (video card).
	IDXGIAdapter* adapter;
	result = factory->EnumAdapters(0, &adapter);
	if(FAILED(result))
		return false;

	// Enumerate the primary adapter output (monitor).
	IDXGIOutput* adapterOutput;
	result = adapter->EnumOutputs(0, &adapterOutput);
	if(FAILED(result))
		return false;

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)
	UINT numModes{};
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if(FAILED(result))
		return false;

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	DXGI_MODE_DESC* displayModeList;
	displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList)
		return false;

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(result))
		return false;

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	UINT numerator{};
	UINT denominator{};
	for(int i = 0; i < numModes; i++)
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
	result = adapter->GetDesc(&adapterDesc);
	if(FAILED(result))
		return false;

	// Store the dedicated video card memory in megabytes.
	_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	size_t stringLength;
	int error = wcstombs_s(&stringLength, _videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		OutputDebugStringA("Video card description failed.");
		return false;
	}
		

	// Release the display mode list.
	delete [] displayModeList;
	displayModeList = nullptr;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = nullptr;

	// Release the adapter.
	adapter->Release();
	adapter = nullptr;

	// Release the factory.
	factory->Release();
	factory = nullptr;

	// Initialize the swap chain description.
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    swapChainDesc.BufferCount = 1;	// Double buffered
    swapChainDesc.BufferDesc.Width = windowWidth;
    swapChainDesc.BufferDesc.Height = windowHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// Set regular 32-bit surface for the back buffer.

	// Set the refresh rate of the back buffer.
	if(_VSyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
	    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// Set the usage of the back buffer.
    swapChainDesc.OutputWindow = hwnd;								// Set the handle for the window to render to.
    swapChainDesc.SampleDesc.Count = 1;								// Turn multisampling off.
    swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !fullscreen;							// Set to full screen or windowed mode.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;		// Discard the back buffer contents after presenting.
	swapChainDesc.Flags = 0;									// Don't set the advanced flags.
	
	D3D_FEATURE_LEVEL featureLevel;
	featureLevel = D3D_FEATURE_LEVEL_11_1;						// Set the feature level to DirectX 11.

	// Create the swap chain, Direct3D device, and Direct3D device context. D3D11_CREATE_DEVICE_DEBUG useful for now, not in release
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1,
										   D3D11_SDK_VERSION, &swapChainDesc, &_swapChain, &_device, NULL, &_deviceContext);
	if(FAILED(result))
		return false;





	// Get the pointer to the back buffer.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBufferPtr;
	result = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if(FAILED(result))
		return false;
	
	// Create the render target view with the back buffer pointer. Uknown format means adjust to parent.
	_renderTarget.fromExistingTexture(_device, backBufferPtr.Get(), windowWidth, windowHeight);

	// Depth stencil states - these are just hardcoded right now as I don't need anything special.
	DepthStencilDesc depthStencilDesc{};
	depthStencilDesc.depthFunc = DepthStencilDesc::COMP::LESS_EQUAL;
	_depthStencilLessEquals.createDepthStencilState(_device, depthStencilDesc);

	depthStencilDesc.depthFunc = DepthStencilDesc::COMP::LESS;
	_depthStencilLess.createDepthStencilState(_device, depthStencilDesc);

	depthStencilDesc.depthTest = false;
	depthStencilDesc.stencilTest = false;
	depthStencilDesc.depthFunc = DepthStencilDesc::COMP::ALWAYS;
	_depthStencilNoDepthTest.createDepthStencilState(_device, depthStencilDesc);

	// Set the depth stencil state.
	//_depthStencilLess.bind(_deviceContext);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	_renderTarget.bind(_deviceContext);
	_depthStencilLessEquals.bind(_deviceContext);

	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc{};
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	if (FAILED(_device->CreateRasterizerState(&rasterDesc, &_r_s_solid_cull)))
		return false;

	rasterDesc.CullMode = D3D11_CULL_NONE;
	if (FAILED(_device->CreateRasterizerState(&rasterDesc, &_r_s_solid_no_cull)))
		return false;

	rasterDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	if (FAILED(_device->CreateRasterizerState(&rasterDesc, &_r_s_wireframe)))
		return false;

	// Now set the rasterizer state.
	_deviceContext->RSSetState(_r_s_solid_cull);

	//blending code @TODO see why it messes with texture.... turn off until then
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; //D3D11_COLOR_WRITE_ENABLE_ALL;

	result = _device->CreateBlendState(&blendDesc, &_blendState);
	if (FAILED(result))
		return false;

	// Modify the description to create an alpha disabled blend state description.
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	// Create the blend state using the description.
	result = _device->CreateBlendState(&blendDesc, &_noBlendState);
	if (FAILED(result))
		return false;
	
	_viewport._viewport = Viewport::CreateViewport((float)windowWidth, (float)windowHeight);

	_deviceContext->RSSetViewports(1, &_viewport._viewport);

    return true;
}


void D3D::Shutdown()
{
	if(_swapChain)
		_swapChain->SetFullscreenState(false, NULL);

	if(_r_s_solid_cull){
		_r_s_solid_cull->Release();
		_r_s_solid_cull = nullptr;
	}

	if(_deviceContext){
		_deviceContext->Release();
		_deviceContext = nullptr;
	}

	if(_device){
		_device->Release();
		_device = nullptr;
	}

	if(_swapChain){
		_swapChain->Release();
		_swapChain = nullptr;
	}

	return;
}


void D3D::ClearColourDepthBuffers()
{
	_renderTarget.clear(_deviceContext);
}


void D3D::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if(_VSyncEnabled)
		_swapChain->Present(1, 0);	// Lock to screen refresh rate.
	else
		_swapChain->Present(0, 0);	// Present as fast as possible.
}


ID3D11Device* D3D::GetDevice(){ return _device; }


ID3D11DeviceContext* D3D::GetDeviceContext(){ return _deviceContext; }


void D3D::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, _videoCardDescription);
	memory = _videoCardMemory;
}


void D3D::SetBackBufferRenderTarget()
{
	_deviceContext->RSSetViewports(1, &_viewport._viewport);
	_renderTarget.bind(_deviceContext);
	ClearColourDepthBuffers();
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
	_deviceContext->RSSetState(_r_s_solid_cull);
}


void D3D::setRSSolidNoCull()
{
	_deviceContext->RSSetState(_r_s_solid_no_cull);
}


void D3D::setRSWireframe()
{
	_deviceContext->RSSetState(_r_s_wireframe);
}

void D3D::setDSSNoTest()
{
	_depthStencilNoDepthTest.bind(_deviceContext);
}


void D3D::setDSSLessEquals()
{
	_depthStencilLessEquals.bind(_deviceContext);
}


void D3D::setDSSLess()
{
	_depthStencilLess.bind(_deviceContext);
}