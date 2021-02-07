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
	void createDepthStencil(ID3D11Device* device, UINT w, UINT h, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT, UINT extraFlags = 0u, UINT arrSize = 1u)
	{
		createDepthStencilBuffer(device, w, h, arrSize, format, extraFlags);

		createDepthStencilStateDesc(device);

		createDepthStencilViews(device, arrSize, format, extraFlags);
	}

	void clearView(ID3D11DeviceContext* context, uint16_t index = 0u, float depthVal = 1.f, uint8_t stencilVal = 0u)
	{
		context->ClearDepthStencilView(_dsvs[index].Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 
			depthVal, stencilVal);
	}


	ID3D11DepthStencilView* dsvPtr(uint16_t index = 0u)
	{
		return _dsvs[index].Get();
	}

private:

	Microsoft::WRL::ComPtr<ID3D11Texture2D> _dsb{nullptr};
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _dss{ nullptr };
	// One dsv for every item in the array
	std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> _dsvs;

	// If the stencil is an array of stencils on the gpu, this is an view over the whole array
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _arraySrv;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _readOnlyDsv{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _debugSrv{ nullptr };



	void createDepthStencilBuffer(ID3D11Device* device, UINT w, UINT h, UINT arrSize, DXGI_FORMAT format, UINT extraFlags)
	{
		// Depth stencil buffer - basically just a texture all in all
		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		depthBufferDesc.Width = w;
		depthBufferDesc.Height = h;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = arrSize;
		depthBufferDesc.Format = format;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | extraFlags;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		if (FAILED(device->CreateTexture2D(&depthBufferDesc, nullptr, &_dsb)))
		{
			DebugBreak();
		}
	}


	void createDepthStencilStateDesc(ID3D11Device* device)
	{
		// Depth stencil state description and creation
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
		if (FAILED(device->CreateDepthStencilState(&depthStencilDesc, &_dss)))
		{
			DebugBreak();
		}
	}


	void createDepthStencilViews(ID3D11Device* device, UINT arrSize, DXGI_FORMAT format, UINT extraFlags)
	{
		// Create views, one for each depth stencil in array (although it's usually 1)
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = DXGI_FORMAT_UNKNOWN;	// DXGI_FORMAT_UNKNOWN should match the owning buffer's format
		depthStencilViewDesc.Flags = 0;

		_dsvs.resize(arrSize);

		// Note to self: Don't try to simplify to a for loop
		// D3D11_DSV_DIMENSION_TEXTURE2D desc has no ".Texture2DArray" member since it's a union, easy to forget
		if (arrSize == 1u)
		{
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;

			if (FAILED(device->CreateDepthStencilView(_dsb.Get(), &depthStencilViewDesc, &_dsvs[0])))
			{
				DebugBreak();
			}
		}
		else
		{
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			depthStencilViewDesc.Texture2DArray.ArraySize = 1;
			depthStencilViewDesc.Texture2DArray.MipSlice = 0;

			for (uint8_t i = 0; i < arrSize; ++i)
			{
				depthStencilViewDesc.Texture2DArray.FirstArraySlice = i;

				if (FAILED(device->CreateDepthStencilView(_dsb.Get(), &depthStencilViewDesc, &_dsvs[i])))
				{
					DebugBreak();
				}
			}
			depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0; // Reset for read_only view
		}


		// Create a read-only view for debugging purposes. It will either use the single buffer version or first buffer in the array
		depthStencilViewDesc.Flags |= D3D11_DSV_READ_ONLY_DEPTH;

		if (format == DXGI_FORMAT_D24_UNORM_S8_UINT || format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
			depthStencilViewDesc.Flags |= D3D11_DSV_READ_ONLY_STENCIL;

		if (FAILED(device->CreateDepthStencilView(_dsb.Get(), &depthStencilViewDesc, &_readOnlyDsv)))
		{
			OutputDebugStringA("Failed to create read only depth stencil view (CSM). \n");
			DebugBreak();
		}


		// Create shader resource view if required
		if (extraFlags & D3D11_BIND_SHADER_RESOURCE == D3D11_BIND_SHADER_RESOURCE)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = format;

			if (arrSize == 1)
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1u;
				srvDesc.Texture2D.MostDetailedMip = 0u;
			}
			else
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.ArraySize = arrSize;
				srvDesc.Texture2DArray.FirstArraySlice = 0u;
				srvDesc.Texture2DArray.MipLevels = 1u;
				srvDesc.Texture2DArray.MostDetailedMip = 0u;
			}

			if (FAILED(device->CreateShaderResourceView(_dsb.Get(), &srvDesc, &_arraySrv)))
			{
				OutputDebugStringA("Can't create shader resource view. (CSM) \n");
				DebugBreak();
			}
		}

		// For debugging with ImGUI, I think it doesn't support views to arrays
		/*
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc2;
		srvDesc2.Format = DXGI_FORMAT_R32_FLOAT;

		if (arrSize == 1)
		{
			srvDesc2.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		}
		else
		{
			srvDesc2.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
		}
		srvDesc2.Texture2D.MipLevels = 1u;
		srvDesc2.Texture2D.MostDetailedMip = 0u;
		srvDesc2.

		if (FAILED(device->CreateShaderResourceView(_shadowMapArray, &srvDesc2, &_debugSrv)))
		{
			OutputDebugStringA("Can't create debug shader resource view. (CSM) \n");
		}
		*/
	}
};