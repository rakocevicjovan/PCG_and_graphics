#pragma once
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <dxgi.h>
#include <d3d11_4.h>
#include <d3dcommon.h>
#include "RenderTarget.h"
#include "SwapChain.h"
#include "Viewport.h"
#include "Math.h"


class D3D
{
public:

	bool initialize(int, int, bool, HWND, bool);
	
	void ClearColourDepthBuffers();
	void EndScene();

	void GetVideoCardInfo(char*, int&);
	void SetBackBufferRenderTarget();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

	void setRSSolidCull();
	void setRSSolidNoCull();
	void setRSWireframe();

	void setDSSNoTest();
	void setDSSLessEquals();
	void setDSSLess();

	inline ID3D11Device* getDevice() { return _device.Get(); }
	inline ID3D11DeviceContext* getContext() { return _deviceContext.Get(); }

	RenderTarget _renderTarget;
	Viewport _viewport;

private:
	bool _VSyncEnabled;
	int _videoCardMemory;
	char _videoCardDescription[128];
	SwapChain _swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device5> _device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext4> _deviceContext;

	DepthStencilState _depthStencilLess;
	DepthStencilState _depthStencilLessEquals;
	DepthStencilState _depthStencilNoDepthTest;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> _r_s_solid_cull;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>_r_s_solid_no_cull;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> _r_s_wireframe;

	ID3D11BlendState* _blendState, *_noBlendState;
};