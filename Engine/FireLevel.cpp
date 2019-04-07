#include "FireLevel.h"



void FireLevel::init(Systems& sys)
{
	_sys._colEngine.registerController(_sys._controller);

	//hexer initialization
	float hexRadius = 30.f;
	Procedural::Geometry hex;
	hex.GenHexaprism(hexRadius, 10.f);
	hexModel.meshes.push_back(Mesh(hex, device));
	hexer.init(hexRadius);
	sys._renderer.setCameraMatrix(SMatrix::CreateTranslation(hexer._points[0]));

	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(10.f));
	Math::Translate(will.transform, SVec3(768, 138, 768));

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(333.f, 666.f, 999.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)

	/*
	//terrain generation

	Texture terrainTex;
	auto fltVec = terrainTex.generateRidgey(256, 256, 0.f, 1.61803f, 0.5793f, 1.f, 6u);	//auto fltVec = tempTex.generateTurbulent(256, 256, 1.f, 1.61803, 0.5793f, 6u);
	terrain.setScales(4, 100, 4);
	terrain.GenFromTexture(terrainTex.w, terrainTex.h, fltVec);
	terrain.Mesa(SVec2(512), 384, 128, -256);
	terrain.CircleOfScorn(SVec2(768, 768), 40.f, PI * 0.01337f, 2 * PI, 64, 1.2 * PI);
	terrain.setOffset(0, 128, 0);

	terrain.setTextureData(device, 10, 10, { "../Textures/LavaCracks/diffuse.png", "../Textures/LavaCracks/normal.png" });
	terrain.SetUp(device);
	
	//other items
	Procedural::Terrain island = Procedural::Terrain(128, 128, SVec3(4, 300, 4));
	island.Mesa(SVec2(256), 32, 64, 128);
	island.setTextureData(device, 10, 10, { "../Textures/Lava/diffuse.jpg", "../Textures/Lava/normal.jpg" });

	
	for (auto& pos : hexer._points)
	{
		island.setOffset(pos.x - 256.f, 0, pos.z - 256.f);
		island.SetUp(device);
		_islands.push_back(island);
	}*/

	lavaSheet = Procedural::Terrain(256, 256, SVec3(4, 2, 4));
	lavaSheet.setTextureData(device, 10, 10, { "../Textures/LavaIntense/diffuse.jpg", "../Textures/LavaIntense/normal.jpg" });
	lavaSheet.setOffset(0, 32, 0);
	lavaSheet.CalculateTexCoords();
	lavaSheet.CalculateNormals();

	lavaSheetModel = Model(lavaSheet, device);

	//textures
	hexDiffuseMap.LoadFromFile("../Textures/Crymetal/diffuse.jpg");
	hexDiffuseMap.Setup(device);
	hexNormalMap.LoadFromFile("../Textures/Crymetal/normal.jpg");
	hexNormalMap.Setup(device);
}



void FireLevel::procGen()
{
	if (isTerGenerated)
		return;

	Procedural::Geometry hex;
	std::vector<Procedural::Geometry> hexes = hex.GenHexGrid(30.f, 10.f, 2);
	for (auto& h : hexes)
		hexCluster.meshes.emplace_back(h, device);

	isTerGenerated = true;
}



void FireLevel::draw(const RenderContext& rc)
{
	setUpCollision();
	processInput(rc.dTime);
	updateCam(rc.dTime);
	hexer.update(rc.dTime);

	SVec3 potentialPlatformPos;
	if (hexer.marchTowardsPoint(potentialPlatformPos))
	{
		hexer._platforms.push_back(Platform(potentialPlatformPos, &hexModel, &_sys._renderer._shMan.shaderNormalMaps));
		//_sys._colEngine.registerActor(hexer._platforms.back().actor, BoundingVolumeType::BVT_AABB);
	}

	dc->RSSetViewports(1, &rc.d3d->viewport);				//use default viewport for output dimensions
	rc.d3d->SetBackBufferRenderTarget();					//set default screen buffer as output target
	rc.d3d->BeginScene(rc.d3d->clearColour);				//clear colour and depth buffer

	/*
	terrain.Draw(dc, rc.shMan->shaderTerNorm, *rc.cam, pointLight, rc.elapsed);

	for (auto& island : _islands) 
	{
		island.Draw(dc, rc.shMan->shaderTerNorm, *rc.cam, pointLight, rc.elapsed);
	}
	*/

	for (Platform p : hexer._platforms)
	{
		if (!p.active)
			continue;

		hexModel.transform = p.actor.transform;
		rc.shMan->shaderNormalMaps.SetShaderParameters(dc, hexModel, *rc.cam, pointLight, rc.dTime, hexDiffuseMap, hexNormalMap);
		hexModel.Draw(dc, rc.shMan->shaderNormalMaps);
	}

	rc.d3d->TurnOffCulling();
	rc.d3d->SwitchDepthToLessEquals();
	rc.shMan->shaderSkybox.SetShaderParameters(dc, skybox.transform, *rc.cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(dc, rc.shMan->shaderSkybox);
	rc.shMan->shaderSkybox.ReleaseShaderParameters(dc);
	rc.d3d->SwitchDepthToDefault();
	rc.d3d->TurnOnCulling();


	//transparent items
	rc.d3d->TurnOnAlphaBlending();

	rc.shMan->shVolumLava.SetShaderParameters(dc, lavaSheetModel, *rc.cam, rc.elapsed);
	lavaSheetModel.Draw(dc, rc.shMan->shVolumLava);

	rc.shMan->shVolumFire.SetShaderParameters(dc, will, *rc.cam, rc.elapsed);
	will.Draw(dc, rc.shMan->shVolumFire);

	rc.d3d->TurnOffAlphaBlending();

	//finish up
	rc.d3d->EndScene();

	resetCollision();
}



void FireLevel::setUpCollision()
{
	for (Platform& platform : hexer._platforms)
	{
		Collider c;
		c.BVT = BVT_AABB;
		c.actParent = &platform.actor;
		c.dynamic = true;
		
		for (Mesh m : platform.actor.gc.model->meshes)
			c.hulls.push_back(_sys._colEngine.genBoxHull(&m));

		for (Hull* h : c.hulls)
		{
			SVec4 daddyPos = c.actParent->transform.Translation();
			SVec3 kiddyPos = SVec3(daddyPos.x, daddyPos.y, daddyPos.z);
			h->setPosition(kiddyPos);
		}

		_sys._colEngine.addToGrid(&c);

		_levelColliders.push_back(c);
	}


	
}



void FireLevel::resetCollision()
{
	for (Collider& collider : _levelColliders)
	{
		_sys._colEngine.removeFromGrid(collider);
		collider.ReleaseMemory();
	}

	_levelColliders.clear();
}



bool FireLevel::processInput(float dTime)
{
	ProcessSpecialInput(dTime);

	if (_sys._inputManager.IsKeyDown((short)'M') && sinceLastInput > .33f)
	{
		return true;
		sinceLastInput = 0;
	}

	return false;
}