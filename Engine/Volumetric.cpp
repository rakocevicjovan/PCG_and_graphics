#include "Volumetric.h"

namespace Procedural
{
	Volumetric::Volumetric()
	{
	};

	Volumetric::~Volumetric()
	{
	};



	void Volumetric::init(UINT w, UINT h, UINT d)
	{
		_w = w;
		_h = h;
		_d = d;

		_volume.resize(_w * _h * _d);

		for (int x = 0; x < _w; ++x)
		{
			for (int y = 0; y < _h; ++y)
			{
				for (int z = 0; z < _d; ++z)
				{
					UINT index = access(x, y, z);
					_volume[index].pos = SVec3(x, y, z);
				}
			}
			
		}
	}



	inline UINT Volumetric::access(UINT x, UINT y, UINT z)
	{
		UINT index;
		index = x * _h * _d + y * _d + z;
		return index;
	}



	void Volumetric::petrurb()
	{
		for (auto& v : _volume)
		{
			float col = Texture::Turbulence3D(v.pos.x, v.pos.y, v.pos.z, 2.013759, 0.49739, 4);
			v.col = SVec4(col, col, col, 1.0f);
		}
	}



	void Volumetric::Create3DTexture(ID3D11Device* device)
	{
		D3D11_TEXTURE3D_DESC tex3Desc;
		D3D11_SUBRESOURCE_DATA tex3Data;
		D3D11_SHADER_RESOURCE_VIEW_DESC srv3Desc;

		tex3Desc.Width = _w;
		tex3Desc.Height = _h;
		tex3Desc.Depth = _d;
		tex3Desc.MipLevels = 1;
		tex3Desc.Format = DXGI_FORMAT_R32_FLOAT;
		tex3Desc.Usage = D3D11_USAGE_IMMUTABLE;
		tex3Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tex3Desc.CPUAccessFlags = 0;
		tex3Desc.MiscFlags = 0;

		tex3Data.pSysMem = (void *)_volume.data();
		tex3Data.SysMemPitch = sizeof(Voxel) * _volume.size();
		tex3Data.SysMemSlicePitch = 0;
		
		if (FAILED(device->CreateTexture3D(&tex3Desc, &tex3Data, &_tex3d)))
		{
			OutputDebugStringA("Can't create texture3d. \n");
			exit(333);
		}

		srv3Desc.Format = tex3Desc.Format;
		srv3Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		srv3Desc.Texture3D.MostDetailedMip = 0;
		srv3Desc.Texture3D.MipLevels = 1;

		if (FAILED(device->CreateShaderResourceView(_tex3d, &srv3Desc, &_srv)))
		{
			OutputDebugStringA("Can't create shader resource view. \n");
			exit(43);
		}
	}

}