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
			std::vector<float> yeetFloat;
			size_t sheetSize = 128 * 128 * 4;
			size_t yeetSize = 128 * sheetSize;
			yeetFloat.reserve(yeetSize);
			
			std::vector<float> flVec;
			flVec.reserve(sheetSize);

			//not really optimal but it's quite fast still
			for (int i = 0; i < 128; ++i)
			{
				std::stringstream ss;
				ss << std::setw(3) << std::setfill('0') << (i + 1);
				flVec = Texture::GetFloatsFromFile("../Textures/Generated/my3DTextureArray." + ss.str() + ".tga");
				yeetFloat.insert(yeetFloat.end(), flVec.begin(), flVec.end());
			}

			desc.Width = 128;
			desc.Height = 128;
			desc.Depth = 128;
			desc.MipLevels = 8;
			desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

			//D3D11_SUBRESOURCE_DATA texData;
			std::vector<D3D11_SUBRESOURCE_DATA> texData(desc.MipLevels);

			int denominator = 1;
			float texelByteWidth = 4 * sizeof(float);	//RGBA format with each being a float32

			for (int i = 0; i < desc.MipLevels; ++i)
			{
				texData[i].pSysMem = (void *)yeetFloat.data();
				texData[i].SysMemPitch = (desc.Width / denominator) * texelByteWidth;
				texData[i].SysMemSlicePitch = texData[i].SysMemPitch * (desc.Height / denominator);
				//denominator *= 2;
			}
			
			HRESULT hr = device->CreateTexture3D(&desc, &texData[0], &texId);
			if (FAILED(hr))
			{
				OutputDebugStringA("Can't create texture3d. \n");
				exit(42);
			}

			shaderResourceViewDesc.Format = desc.Format;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture3D.MipLevels = desc.MipLevels;

			if (FAILED(device->CreateShaderResourceView(texId, &shaderResourceViewDesc, &srv)))
			{
				OutputDebugStringA("Can't create shader resource view. \n");
				exit(43);
			}

			context->GenerateMips(srv); //gdi directX... can't you be convenient ONCE

			return true;
		}
	};

}