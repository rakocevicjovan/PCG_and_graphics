#pragma once
#include <d3d11.h>
#include <wrl/client.h>



class DepthStencil
{
public:

	DepthStencil() {}

	DepthStencil(ID3D11Device* device, UINT w, UINT h, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT)
	{
		createDepthStencil(device, w, h, format);
	}

	// @TODO add more 
	void createDepthStencil(ID3D11Device* device, UINT w, UINT h, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT)
	{
		// Depth stencil buffer - basically just a texture all in all
		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		depthBufferDesc.Width = w;
		depthBufferDesc.Height = h;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = format;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		// Create the texture for the depth buffer using the filled out description.
		if (FAILED(device->CreateTexture2D(&depthBufferDesc, nullptr, &_dsb)))
		{
			DebugBreak();
		}

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing.
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing.
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create the depth stencil state.
		if (FAILED(device->CreateDepthStencilState(&depthStencilDesc, &_dss)))
		{
			DebugBreak();
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = format;	// DXGI_FORMAT_UNKNOWN matches the parent buffer
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		depthStencilViewDesc.Flags = 0;

		// Create a depth stencil view
		if (FAILED(device->CreateDepthStencilView(_dsb.Get(), &depthStencilViewDesc, &_dsv)))
		{
			DebugBreak();
		}
	}

	void clear(ID3D11DeviceContext* context, float depthVal = 1.f, uint8_t stencilVal = 0u)
	{
		context->ClearDepthStencilView(_dsv.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 
			depthVal, stencilVal);
	}

	ID3D11DepthStencilView* dsvPtr()
	{
		return _dsv.Get();
	}
private:

	Microsoft::WRL::ComPtr<ID3D11Texture2D> _dsb{nullptr};
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _dss{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _dsv{ nullptr };
};