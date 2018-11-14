#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include "Math.h"

class D3DClass{

public:
	D3DClass();
	~D3DClass();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();
	
	void BeginScene(float*);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetVideoCardInfo(char*, int&);
	ID3D11DepthStencilView* GetDepthStencilView();
	void SetBackBufferRenderTarget();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

	void TurnOnCulling();
	void TurnOffCulling();

	void SwitchDepthToLessEquals();
	void SwitchDepthToDefault();

	float _fieldOfView, _screenAspect;
	ID3D11RenderTargetView* m_renderTargetView;
	D3D11_VIEWPORT viewport;

private:
	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];
	IDXGISwapChain* m_swapChain;
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState, *DSLessEqual;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState, *m_rasterStateNoCull;
	ID3D11BlendState* m_blendState, *m_noBlendState;
};

#endif