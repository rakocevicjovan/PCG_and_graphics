#pragma once
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11_4.h>
#include "Viewport.h"
#include "Math.h"

class D3D
{

public:
	D3D();
	~D3D();

	bool Initialize(int, int, bool, HWND, bool);
	void Shutdown();
	
	void ClearColourDepthBuffers();
	void EndScene();

	void GetVideoCardInfo(char*, int&);
	ID3D11DepthStencilView* GetDepthStencilView();
	void SetBackBufferRenderTarget();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

	void setRSSolidCull();
	void setRSSolidNoCull();
	void setRSWireframe();

	void setDSSNoTest();
	void setDSSLessEquals();
	void setDSSLess();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	ID3D11RenderTargetView* _renderTargetView;
	D3D11_VIEWPORT _viewport;
	float _clearColour[4] = { 0.2f, 0.1f, 0.3f, 1.0f };

private:
	bool _vsync_enabled;
	int _videoCardMemory;
	char _videoCardDescription[128];
	IDXGISwapChain* _swapChain;
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	ID3D11Texture2D* _depthStencilBuffer;
	ID3D11DepthStencilState* _depthStencilLess, *_depthStencilLessEquals, *_depthStencilNoDepthTest;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11RasterizerState* _r_s_solid_cull, *_r_s_solid_no_cull, *_r_s_wireframe;
	ID3D11BlendState* _blendState, *_noBlendState;
};