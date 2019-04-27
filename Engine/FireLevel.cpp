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
	randy.setCameraMatrix(SMatrix::CreateTranslation(hexer._points[0]));

	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(10.f));
	Math::Translate(will.transform, SVec3(768, 138, 768));
	goal = will.transform.Translation();

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
	Procedural::Terrain island = Procedural::Terrain(64, 64, SVec3(16, 300, 16));
	island.Mesa(SVec2(256), 32, 64, 64);
	island.setTextureData(device, 10, 10, { "../Textures/Lava/diffuse.jpg", "../Textures/Lava/normal.jpg" });
	island.CalculateNormals();
	island.CalculateTexCoords();
	islandModel = Model(island, device);


	lavaSheet = Procedural::Terrain(2, 2, SVec3(1024, 1, 1024));
	lavaSheet.setOffset(0, 32, 0);
	lavaSheet.CalculateTexCoords();
	lavaSheet.CalculateNormals();

	lavaSheetModel = Model(lavaSheet, device);

	//textures
	hexDiffuseMap.LoadFromFile("../Textures/Crymetal/diffuse.jpg");
	hexDiffuseMap.Setup(device);
	hexNormalMap.LoadFromFile("../Textures/Crymetal/normal.jpg");
	hexNormalMap.Setup(device);

	Procedural::LSystem linden;
	linden.reseed("F");
	linden.addRule('F', "FF+[+*F-F-/F]*-[-F/+F+*F]/");
	linden.rewrite(4);
	float liangle = PI * 0.138888f;		//liangle = PI * .5f;
	tree = linden.genModel(device, 12.f, 3.f, .77f, .77f, liangle, liangle);
	tree.transform = SMatrix::CreateFromAxisAngle(SVec3(0, 0, 1), PI * 0.5);
	tree.transform *= SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), -PI * 0.4);
	tree.transform *= SMatrix::CreateTranslation(860, 60, 800);
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



void FireLevel::update(const RenderContext & rc)
{
	setUpCollision();
	processInput(rc.dTime);
	updateCam(rc.dTime);
	hexer.update(rc.dTime);

	SVec3 potentialPlatformPos;
	if (hexer.marchTowardsPoint(potentialPlatformPos))
		hexer._platforms.push_back(Platform(potentialPlatformPos, &hexModel, &_sys._renderer._shMan.normalMapper));

	if (!_sys._controller.isFlying())
	{
		SVec3 oldPos = _sys._renderer._cam.GetCameraMatrix().Translation();
		float newHeight = terrain.getHeightAtPosition(rc.cam->GetPosition());
		if (rc.cam->GetPosition().y < newHeight)
			rc.cam->SetTranslation(SVec3(oldPos.x, newHeight, oldPos.z));
	}

	win(rc.cam->GetPosition());
}



void FireLevel::draw(const RenderContext& rc)
{
	sceneTex.SetRenderTarget(context);

	terrain.Draw(context, shady.terrainNormals, *rc.cam, pointLight, rc.elapsed);

		
	for (auto& pos : hexer._points)
	{
		shady.terrainNormals.SetShaderParameters(context, SMatrix::CreateTranslation(pos.x - 256.f, 0, pos.z - 256.f), *rc.cam, pointLight, rc.dTime);
		islandModel.Draw(context, shady.terrainNormals);
	}
	
	for (Platform p : hexer._platforms)
	{
		if (!p.active) continue;

		hexModel.transform = p.actor.transform;
		shady.normalMapper.SetShaderParameters(context, hexModel, *rc.cam, pointLight, rc.dTime, hexDiffuseMap, hexNormalMap);
		hexModel.Draw(context, shady.normalMapper);
	}

	shady.normalMapper.SetShaderParameters(context, tree, *rc.cam, pointLight, rc.dTime, tree.textures_loaded[0], tree.textures_loaded[1]);
	tree.Draw(context, shady.normalMapper);

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
	rc.d3d->SetBackBufferRenderTarget();						//set default screen buffer as output target

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
			c.hulls.push_back(collision.genBoxHull(&m, SMatrix::Identity));

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