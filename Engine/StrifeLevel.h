#pragma once
#include "Level.h"
#include "ToolGUI.h"
#include "Terrain.h"
#include "CloudscapeDefinition.h"

#include <sstream>
#include <iomanip>


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

		bool Create3D(ID3D11Device* dvc)
		{
			std::vector<unsigned char> yeet;
			std::vector<float> yeetFloat;
			size_t yeetSize = 128 * 128 * 128 * 4;
			yeet.reserve(yeetSize);
			
			Texture tt;
			for (int i = 0; i < 128; ++i)
			{
				std::stringstream ss;
				ss << std::setw(3) << std::setfill('0') << (i + 1);
				tt.LoadFromFile("../Textures/Generated/my3DTextureArray." + ss.str() + ".tga");
				yeet.insert(yeet.end(), &tt.data[0], &tt.data[tt.w * tt.h * tt.n]);
			}
			
			for (int i = 0; i < yeetSize; ++i)
				yeetFloat.emplace_back( ((float)yeet[i]) / 255.f);

			desc.Width = 128;
			desc.Height = 128;
			desc.Depth = 128;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R8G8B8A8_UNORM
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA texData;

			texData.pSysMem = (void *)yeetFloat.data();
			texData.SysMemPitch = desc.Width * 4 * 4;
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