#include "StrifeLevel.h"
#include "Math.h"
#include "ShaderUtils.h"

namespace Strife
{

	void StrifeLevel::init(Systems& sys)
	{
		skyboxCubeMapper.LoadFromFiles(S_DEVICE, "../Textures/night.dds");

		sceneTex.Init(S_DEVICE, _sys.getWinW() / 2, _sys.getWinH() / 2);
		screenRectangleNode = postProcessor.AddUINODE(S_DEVICE, postProcessor.getRoot(), SVec2(0, 0), SVec2(1, 1), .999999f);

		skybox.LoadModel(S_DEVICE, "../Models/Skysphere.fbx");

		Mesh scrQuadMesh = Mesh(SVec2(0., 0.), SVec2(1.f, 1.f), S_DEVICE, .999999f);	//1.777777f
		screenQuad.meshes.push_back(scrQuadMesh);

		_sys._renderer._cam.SetProjectionMatrix(DirectX::XMMatrixPerspectiveFovLH(0.5 * PI, S_RANDY.getAR(), 1.f, 1000.f));

		//32000.f in lux but that doesn't work... not sure how to do any of this
		LightData lightData(SVec3(1.f, 1.f, 1.f), 1.f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);

		float edge = 256;
		Procedural::Terrain terrain(2, 2, SVec3(edge, 1, edge));
		terrain.setOffset(-edge * .5, 0.f, -edge * .5);
		terrain.CalculateNormals();

		floor = Model(terrain, S_DEVICE);
		floor.transform = SMatrix::CreateTranslation(terrain.getOffset());


		//set up initial cloud definition - most of them can be changed through the gui later
		csDef.celestial = PointLight(lightData, SVec4(0., 999., 999., 1.0f));
		csDef.celestial.alc = SVec3(1., 1., 1.);
		csDef.rgb_sig_absorption = SVec3(.05f);	//SVec3(1.); quite a few can work tbh
		csDef.eccentricity = 0.66f;
		csDef.globalCoverage = .5f;
		csDef.scrQuadOffset = 1.f;
		csDef.heightMask = SVec2(1000, 1500);
		csDef.repeat = SVec4(4096.f, 32.f, 4069.f, 1.f);	//density factor in .w
		
		//load 2D textures
		csDef.weather = Texture(S_DEVICE, "../Textures/DensityTypeTexture.png");
		csDef.blue_noise = Texture(S_DEVICE, "../Textures/blue_noise_64_tiled.png");

		///create/load 3D tectures

		//Create3D();
		//CreateFine3D();
		
		CreateFine3DOneChannel();
		Create3DOneChannel();		
		
		csDef.baseVolume = baseSrv;
		csDef.fineVolume = fineSrv;
	}



	void StrifeLevel::procGen(){}



	void StrifeLevel::update(const RenderContext& rc)
	{
		if (_sys._inputManager.isKeyDown((short)'M'))
		{
			S_INMAN.toggleMouseMode();
		}

		if(!S_INMAN.getMouseMode())
			updateCam(rc.dTime);
	}



	void StrifeLevel::draw(const RenderContext& rc)
	{
		rc.d3d->ClearColourDepthBuffers();

		//terrain
		S_SHADY.light.SetShaderParameters(S_CONTEXT, floor.transform, *rc.cam, csDef.celestial, rc.dTime);
		floor.Draw(S_CONTEXT, S_SHADY.light);

		sceneTex.SetRenderTarget(S_CONTEXT);

		rc.d3d->TurnOnAlphaBlending();

		S_SHADY.strife.SetShaderParameters(S_CONTEXT, *rc.cam, csDef, rc.elapsed);
		screenQuad.Draw(S_CONTEXT, S_SHADY.strife);
		S_SHADY.strife.ReleaseShaderParameters(S_CONTEXT);

		rc.d3d->TurnOffAlphaBlending();
		

		S_CONTEXT->RSSetViewports(1, &_sys._D3D.viewport);
		S_CONTEXT->OMSetRenderTargets(1, &_sys._D3D.m_renderTargetView, _sys._D3D.GetDepthStencilView());

		postProcessor.draw(S_CONTEXT, S_SHADY.HUD, sceneTex.srv);
		

		//GUI
		if(S_INMAN.getMouseMode())
			ToolGUI::Render(csDef);

		rc.d3d->EndScene();
	}



	bool StrifeLevel::Create3D()
	{
		D3D11_TEXTURE3D_DESC desc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		int size = 128;

		std::vector<float> yeetFloat;
		size_t sheetSize = size * size * 4;
		size_t yeetSize = size * sheetSize;
		yeetFloat.reserve(yeetSize);

		std::vector<float> flVec;
		flVec.reserve(sheetSize);

		//not really optimal but it's still quite fast
		for (int i = 0; i < size; ++i)
		{
			std::stringstream ss;
			ss << std::setw(3) << std::setfill('0') << (i + 1);
			flVec = Texture::GetFloatsFromFile("../Textures/Generated/my3DTextureArray." + ss.str() + ".tga");
			yeetFloat.insert(yeetFloat.end(), flVec.begin(), flVec.end());
		}

		desc.Width = size;
		desc.Height = size;
		desc.Depth = size;
		desc.MipLevels = 8;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		//D3D11_SUBRESOURCE_DATA texData;
		std::vector<D3D11_SUBRESOURCE_DATA> texData(desc.MipLevels);

		float texelByteWidth = 4 * sizeof(float);	//RGBA format with each being a float32

		for (int i = 0; i < desc.MipLevels; ++i)
		{
			texData[i].pSysMem = (void *)yeetFloat.data();
			texData[i].SysMemPitch = desc.Width * texelByteWidth;
			texData[i].SysMemSlicePitch = texData[i].SysMemPitch * desc.Height;
		}

		HRESULT hr = S_DEVICE->CreateTexture3D(&desc, &texData[0], &baseTexId);
		if (FAILED(hr))
		{
			OutputDebugStringA("Can't create texture3d. \n");
			exit(42);
		}

		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture3D.MipLevels = desc.MipLevels;

		if (FAILED(S_DEVICE->CreateShaderResourceView(baseTexId, &shaderResourceViewDesc, &baseSrv)))
		{
			OutputDebugStringA("Can't create shader resource view. \n");
			exit(43);
		}

		S_CONTEXT->GenerateMips(baseSrv);

		return true;
	}



	bool StrifeLevel::CreateFine3D()
	{
		D3D11_TEXTURE3D_DESC desc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		int size = 32;
		int numFloats = 4;
		int texelByteWidth = numFloats * sizeof(float);

		std::vector<float> floatVector;
		size_t sheetSize = size * size * numFloats;
		size_t volumeFloatSize = size * sheetSize;
		floatVector.reserve(volumeFloatSize);

		float z = 1.f / 32.f;

		//fill out with good ole Perlin fbm
		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < size; ++j)
			{
				for (int k = 0; k < size; ++k)
				{
					//floatVector.emplace_back(fabs(Texture::Perlin3DFBM(i * z, j * z, k * z, 2.f, .5f, 3u)));
					floatVector.emplace_back(Sebh::Cells(z * SVec3(i, j, k),  1));
					floatVector.emplace_back(Sebh::Cells(z * SVec3(i, j, k), 2));
					floatVector.emplace_back(Sebh::Cells(z * SVec3(i, j, k), 3));
					floatVector.emplace_back(0.f);	//dx crying over no 24 bit format so we have this...
				}
			}
		}

		desc.Width = size;
		desc.Height = size;
		desc.Depth = size;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA texData;

		texData.pSysMem = (void *)floatVector.data();
		texData.SysMemPitch = desc.Width * texelByteWidth;
		texData.SysMemSlicePitch = texData.SysMemPitch * desc.Height;

		HRESULT hr = S_DEVICE->CreateTexture3D(&desc, &texData, &fineTexId);
		if (FAILED(hr))
		{
			OutputDebugStringA("Can't create texture3d. \n");
			exit(42);
		}

		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture3D.MipLevels = desc.MipLevels;

		if (FAILED(S_DEVICE->CreateShaderResourceView(fineTexId, &shaderResourceViewDesc, &fineSrv)))
		{
			OutputDebugStringA("Can't create shader resource view. \n");
			exit(43);
		}

		return true;
	}



	bool StrifeLevel::Create3DOneChannel()
	{
		D3D11_TEXTURE3D_DESC desc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		int size = 128;
		int numChannels = 1;

		std::vector<float> finalArray;
		size_t sheetSizeProcessed = size * size * numChannels;
		size_t finalSize = size * sheetSizeProcessed;
		finalArray.reserve(finalSize);

		std::vector<float> flVec;
		size_t sheetSizeInitial = size * size * 4;
		flVec.reserve(sheetSizeInitial);

		std::vector<float> processedVec;
		processedVec.reserve(sheetSizeProcessed);

		//not really optimal but it's still quite fast
		for (int i = 0; i < size; ++i)
		{
			std::stringstream ss;
			ss << std::setw(3) << std::setfill('0') << (i + 1);
			flVec = Texture::GetFloatsFromFile("../Textures/Generated/my3DTextureArray." + ss.str() + ".tga");

			for (int j = 0; j < flVec.size(); j += 4)
			{
				//processedVec.emplace_back(m.x * flVec[j] + m.y * flVec[j + 1] + m.z * flVec[j + 2] + m.w * flVec[j + 3]);
				processedVec.emplace_back(/*Math::clamp(0., 1., */Math::remap(flVec[j + 1] * flVec[j], flVec[j + 2] * .5f, 1., 0., 1.)/*)*/);
				//processedVec.emplace_back(Math::clamp(0., 1., Math::remap(flVec[j], 1.0 - (flVec[j+1] * 0.625 + flVec[j + 2] * 0.25 + flVec[j + 3] * 0.125), 1.0, 0.0, 1.0)));
			}

			finalArray.insert(finalArray.end(), processedVec.begin(), processedVec.end());
			processedVec.clear();
		}

		desc.Width = size;
		desc.Height = size;
		desc.Depth = size;
		desc.MipLevels = 8;
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		//D3D11_SUBRESOURCE_DATA texData;
		std::vector<D3D11_SUBRESOURCE_DATA> texData(desc.MipLevels);

		float texelByteWidth = numChannels * sizeof(float);	//RGBA format with each being a float32

		for (int i = 0; i < desc.MipLevels; ++i)
		{
			texData[i].pSysMem = (void *)finalArray.data();
			texData[i].SysMemPitch = desc.Width * texelByteWidth;
			texData[i].SysMemSlicePitch = texData[i].SysMemPitch * desc.Height;
		}

		HRESULT hr = S_DEVICE->CreateTexture3D(&desc, &texData[0], &baseTexId);
		if (FAILED(hr))
		{
			OutputDebugStringA("Can't create texture3d. \n");
			exit(42);
		}

		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture3D.MipLevels = desc.MipLevels;

		if (FAILED(S_DEVICE->CreateShaderResourceView(baseTexId, &shaderResourceViewDesc, &baseSrv)))
		{
			OutputDebugStringA("Can't create shader resource view. \n");
			exit(43);
		}

		S_CONTEXT->GenerateMips(baseSrv);

		return true;
	}



	bool StrifeLevel::CreateFine3DOneChannel()
	{
		D3D11_TEXTURE3D_DESC desc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		int size = 32;
		int numChannels = 1;
		int texelByteWidth = numChannels * sizeof(float);

		std::vector<float> floatVector;
		size_t numFloatsSheet = size * size * numChannels * 4;
		size_t numFloatsVolume = size * numFloatsSheet;
		floatVector.reserve(numFloatsVolume);

		float z = 1.f / 32.f;

		//fill out with good ole Perlin fbm
		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < size; ++j)
			{
				for (int k = 0; k < size; ++k)
				{
					//floatVector.emplace_back(fabs(Texture::Perlin3DFBM(i * z, j * z, k * z, 2.f, .5f, 3u)));
					floatVector.emplace_back(Sebh::Cells(z * SVec3(i, j, k), 1));
					floatVector.emplace_back(Sebh::Cells(z * SVec3(i, j, k), 2));
					floatVector.emplace_back(Sebh::Cells(z * SVec3(i, j, k), 3));
					floatVector.emplace_back(0.f);	//dx crying over no 24 bit format so we have this...
				}
			}
		}

		std::vector<float> finalVector;
		size_t sheetSizeProcessed = size * size * numChannels;
		size_t finalSizeProcessed = size * sheetSizeProcessed;
		finalVector.reserve(finalSizeProcessed);
		
		for (int i = 0; i < floatVector.size(); i += 4)
		{
			finalVector.emplace_back(2.f * floatVector[i] * floatVector[i + 1] * floatVector[i + 2]);
			//finalVector.emplace_back(.33 * floatVector[i] + .33 * floatVector[i + 1] + .33 * floatVector[i + 2]);
		}

		desc.Width = size;
		desc.Height = size;
		desc.Depth = size;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R32_FLOAT;	//DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA texData;

		texData.pSysMem = (void *)finalVector.data();
		texData.SysMemPitch = desc.Width * texelByteWidth;
		texData.SysMemSlicePitch = texData.SysMemPitch * desc.Height;

		HRESULT hr = S_DEVICE->CreateTexture3D(&desc, &texData, &fineTexId);
		if (FAILED(hr))
		{
			OutputDebugStringA("Can't create texture3d. \n");
			exit(42);
		}

		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture3D.MipLevels = desc.MipLevels;

		if (FAILED(S_DEVICE->CreateShaderResourceView(fineTexId, &shaderResourceViewDesc, &fineSrv)))
		{
			OutputDebugStringA("Can't create shader resource view. \n");
			exit(43);
		}

		return true;
	}
}


//skybox
//randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

//cloudscape, blend into background which depends on the time of the day... or use anything idk...