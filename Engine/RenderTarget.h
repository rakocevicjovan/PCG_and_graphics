#pragma once
#include "Texture.h"
#include "DepthStencil.h"

class RenderTarget
{
public:

	typedef std::underlying_type_t<D3D11_BIND_FLAG> FlagDataType;


	RenderTarget() {}


	// Use shader resource view as an additional flag if required, not included by default.
	RenderTarget(ID3D11Device* device, UINT w, UINT h, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT, FlagDataType additionalFlags = 0)
	{
		D3D11_TEXTURE2D_DESC desc = Texture::create2DTexDesc(w, h, format, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | additionalFlags);
		_tex.createGPUResource(device, &desc, nullptr);

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		if (FAILED(device->CreateRenderTargetView(_tex._dxID.Get(), &renderTargetViewDesc, &_rtv)))
		{
			DebugBreak();
		}

		_depthStencil.createDepthStencil(device, w, h);
	}


	void fromExistingTexture(ID3D11Device* device, ID3D11Texture2D* texture, UINT w, UINT h, FlagDataType additionalFlags = 0)
	{
		_tex._dxID = texture;

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = DXGI_FORMAT_UNKNOWN;	// We can't guarantee knowing parent's format so there it is.
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		if (FAILED(device->CreateRenderTargetView(texture, &renderTargetViewDesc, _rtv.GetAddressOf())))
		{
			__debugbreak();
		}

		_depthStencil.createDepthStencil(device, w, h);
	}


	DepthStencil& getDepthStencil()
	{
		return _depthStencil;
	}


	void bind(ID3D11DeviceContext* context)
	{
		context->OMSetRenderTargets(1, _rtv.GetAddressOf(), _depthStencil._dsvs[0].Get());
	}


	void clear(ID3D11DeviceContext* context)
	{
		context->ClearRenderTargetView(_rtv.Get(), _clearColour);
		_depthStencil.clearView(context);
	}


	static void unbind(ID3D11DeviceContext* context, UINT count = 0)
	{
		context->OMSetRenderTargets(count, nullptr, nullptr);
	}

private:

	Texture _tex;
	DepthStencil _depthStencil;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _rtv{nullptr};

	float _clearColour[4] = {0., 0., 0., 1.};	// Could be a static, however it might be useful and it's a small price to pay
};