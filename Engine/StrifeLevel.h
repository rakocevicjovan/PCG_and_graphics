#pragma once
#include "Level.h"
#include "ToolGUI.h"
#include "Terrain.h"
#include "CloudscapeDefinition.h"

namespace Strife
{

	class StrifeLevel : public Level
	{
	public:
		StrifeLevel(Systems& sys) : Level(sys) {};
		~StrifeLevel() {};

		Model skybox, floor;
		CubeMapper skyboxCubeMapper;

		Model screenQuad;

		CloudscapeDefinition csDef;

		//load and draw all that jazz
		void init(Systems& sys);
		void procGen();
		void update(const RenderContext& rc);
		void draw(const RenderContext& rc);

		void demolish()
		{
			this->~StrifeLevel();
		};

		D3D11_TEXTURE3D_DESC desc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		ID3D11ShaderResourceView* srv;
		ID3D11Texture3D* texId;

		bool Create3D(ID3D11Device* dvc, const SVec3& whd, std::vector<float> data)	//, DXGI_FORMAT format
		{
			std::vector<float> yeet;
			yeet.reserve(64 * 64 * 64);
			Chaos c;
			c.setRange(0., 1.);
			c.fillVector(yeet, yeet.size());

			desc.Width = whd.x;
			desc.Height = whd.y;
			desc.Depth = whd.z;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA texData;

			texData.pSysMem = (void *)yeet.data();
			texData.SysMemPitch = desc.Width * 4;
			texData.SysMemSlicePitch = texData.SysMemPitch * desc.Height;

			if (FAILED(device->CreateTexture3D(&desc, &texData, &texId)))
			{
				OutputDebugStringA("Can't create texture3d. \n");
				exit(42);
			}


			shaderResourceViewDesc.Format = desc.Format;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;

			if (FAILED(device->CreateShaderResourceView(texId, &shaderResourceViewDesc, &srv)))
			{
				OutputDebugStringA("Can't create shader resource view. \n");
				exit(43);
			}

			return true;
		}
	};

}