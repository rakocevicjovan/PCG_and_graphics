#pragma once


struct DepthStencilDesc
{
	// API agnostic
	enum class DW_MASK : bool
	{
		ZERO = 0,
		ALL = 1
	};

	enum class COMP : uint8_t
	{
		NEVER = 1,
		LESS = 2,
		EQUAL = 3,
		LESS_EQUAL = 4,
		GREATER = 5,
		NOT_EQUAL = 6,
		GREATER_EQUAL = 7,
		ALWAYS = 8
	};

	enum class OP : uint8_t
	{
		KEEP = 1,
		ZERO = 2,
		REPLACE = 3,
		INCR_SAT = 4,
		DECR_SAT = 5,
		INVERT = 6,
		INCR = 7,
		DECR = 8
	};

	bool depthTest{ true };
	DW_MASK depthWrite{ DW_MASK::ALL };			//D3D11_DEPTH_WRITE_MASK_ALL allows writing, zero does not
	COMP depthFunc{ COMP::LESS_EQUAL };	// src = incoming data, dst = existing data => do: (src depthFunc dst)

	bool stencilTest{ true };
	uint8_t stencilReadMask{ 0xFF };		// This value is bit-anded when reading the stencil
	uint8_t stencilWriteMask{ 0xFF };		// This value is bit-anded with any write to stencil

	OP FFSFail{ OP::KEEP };			// Stencil failed, pixel discarded, no depth test
	OP FFDFail{ OP::INCR };			// Stencil passed, depth failed
	OP FFSPass{ OP::KEEP };			// Stencil passed, depth passed
	COMP FFSFunc{ COMP::ALWAYS };	// Always pass stencil test

	OP BFSFail{ OP::KEEP };
	OP BFDFail{ OP::DECR };
	OP BFSPass{ OP::KEEP };
	COMP BFSFunc{ COMP::ALWAYS };

	// DX11 specific

	static D3D11_DEPTH_WRITE_MASK toDX(DW_MASK dwm) { return static_cast<D3D11_DEPTH_WRITE_MASK>(dwm); }
	static D3D11_COMPARISON_FUNC toDX(COMP dsc) { return static_cast<D3D11_COMPARISON_FUNC>(dsc); }
	static D3D11_STENCIL_OP toDX(OP dso) { return static_cast<D3D11_STENCIL_OP>(dso); }

	operator D3D11_DEPTH_STENCIL_DESC()
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};

		depthStencilDesc.DepthEnable = depthTest;
		depthStencilDesc.DepthWriteMask = toDX(depthWrite);
		depthStencilDesc.DepthFunc = toDX(depthFunc);

		depthStencilDesc.StencilEnable = stencilTest;
		depthStencilDesc.StencilReadMask = stencilReadMask;
		depthStencilDesc.StencilWriteMask = stencilWriteMask;

		depthStencilDesc.FrontFace.StencilFailOp = toDX(FFSFail);
		depthStencilDesc.FrontFace.StencilDepthFailOp = toDX(FFDFail);
		depthStencilDesc.FrontFace.StencilPassOp = toDX(FFSPass);
		depthStencilDesc.FrontFace.StencilFunc = toDX(FFSFunc);

		depthStencilDesc.BackFace.StencilFailOp = toDX(BFSFail);
		depthStencilDesc.BackFace.StencilDepthFailOp = toDX(BFDFail);
		depthStencilDesc.BackFace.StencilPassOp = toDX(BFSPass);
		depthStencilDesc.BackFace.StencilFunc = toDX(BFSFunc);

		return depthStencilDesc;
	}
};



class DepthStencilState
{
private: 

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _dss{};

public:

	void createDepthStencilState(ID3D11Device* device, DepthStencilDesc desc)
	{
		D3D11_DEPTH_STENCIL_DESC dxdesc = desc;
		if (FAILED(device->CreateDepthStencilState(&dxdesc, &_dss)))
		{
			DebugBreak();
		}
	}

	inline void bind(ID3D11DeviceContext* context, UINT stencilRef = 0u)
	{
		context->OMSetDepthStencilState(_dss.Get(), stencilRef);
	}
};