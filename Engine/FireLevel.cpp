#include "FireLevel.h"



void FireLevel::init(Systems& sys)
{
	collision.registerController(_sys._controller);

	sceneTex.Init(device, _sys.getWinW(), _sys.getWinH());
	brightnessMask.Init(device, _sys.getWinW(), _sys.getWinH());
	blurredTex1.Init(device, _sys.getWinW(), _sys.getWinH());
	blurredTex2.Init(device, _sys.getWinW(), _sys.getWinH());

	screenRectangleNode = postProcessor.AddUINODE(device, postProcessor.getRoot(), SVec2(0, 0), SVec2(1, 1));

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

	
	//terrain generation
	Texture terrainTex;
	auto fltVec = terrainTex.generateRidgey(256, 256, 0.f, 1.61803f, 0.5793f, 1.f, 6u);	//auto fltVec = tempTex.generateTurbulent(256, 256, 1.f, 1.61803, 0.5793f, 6u);
	terrain.setScales(4, 100, 4);
	terrain.GenFromTexture(256, 256, fltVec);
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
	}

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
		hexer._platforms.push_back(Platform(potentialPlatformPos, &hexModel, &_sys._renderer._shMan.normalMapper));

	sceneTex.SetRenderTarget(context);

	terrain.Draw(context, shady.terrainNormals, *rc.cam, pointLight, rc.elapsed);

	for (auto& island : _islands) 
	{
		island.Draw(context, shady.terrainNormals, *rc.cam, pointLight, rc.elapsed);
	}
	

	for (Platform p : hexer._platforms)
	{
		if (!p.active)
			continue;

		hexModel.transform = p.actor.transform;
		shady.normalMapper.SetShaderParameters(context, hexModel, *rc.cam, pointLight, rc.dTime, hexDiffuseMap, hexNormalMap);
		hexModel.Draw(context, shady.normalMapper);
	}

	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	//transparent items
	rc.d3d->TurnOnAlphaBlending();

	shady.shVolumLava.SetShaderParameters(context, lavaSheetModel, *rc.cam, rc.elapsed);
	lavaSheetModel.Draw(context, shady.shVolumLava);

	shady.shVolumFire.SetShaderParameters(context, will, *rc.cam, rc.elapsed);
	will.Draw(context, shady.shVolumFire);

	rc.d3d->TurnOffAlphaBlending();


	//brightnessMask
	brightnessMask.SetRenderTarget(context);
	postProcessor.draw(context, shady.brightnessMasker, sceneTex.srv);

	//blurring horizontally
	blurredTex1.SetRenderTarget(context);
	postProcessor.draw(context, shady.blurHor, brightnessMask.srv);

	//blurring vertically
	blurredTex2.SetRenderTarget(context);
	postProcessor.draw(context, shady.blurVer, blurredTex1.srv);

	for (int i = 0; i < 3; ++i)
	{
		//blurring horizontally
		blurredTex1.SetRenderTarget(context);
		postProcessor.draw(context, shady.blurHor, blurredTex2.srv);

		//blurring vertically
		blurredTex2.SetRenderTarget(context);
		postProcessor.draw(context, shady.blurVer, blurredTex1.srv);
	}
	
	//final scene rendering - the screen quad
	context->RSSetViewports(1, &rc.d3d->viewport);				//use default viewport for output dimensions
	rc.d3d->SetBackBufferRenderTarget();						//set default screen buffer as output target
	rc.d3d->ClearColourDepthBuffers(rc.d3d->clearColour);		//clear colour and depth buffer

	postProcessor.draw(context, shady.bloom, sceneTex.srv, blurredTex2.srv);
	
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
			c.hulls.push_back(collision.genBoxHull(&m));

		for (Hull* h : c.hulls)
		{
			SVec4 daddyPos = c.actParent->transform.Translation();
			SVec3 kiddyPos = SVec3(daddyPos.x, daddyPos.y, daddyPos.z);
			h->setPosition(kiddyPos);
		}

		collision.addToGrid(&c);

		_levelColliders.push_back(c);
	}
}



void FireLevel::resetCollision()
{
	for (Collider& collider : _levelColliders)
	{
		collision.removeFromGrid(collider);
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