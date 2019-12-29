#include "FireLevel.h"
#include "LSystem.h"


void FireLevel::init(Systems& sys)
{
	S_COLLISION.registerController(_sys._defController);

	sceneTex.Init(S_DEVICE, _sys.getWinW(), _sys.getWinH());
	brightnessMask.Init(S_DEVICE, _sys.getWinW(), _sys.getWinH());
	blurredTex1.Init(S_DEVICE, _sys.getWinW(), _sys.getWinH());
	blurredTex2.Init(S_DEVICE, _sys.getWinW(), _sys.getWinH());

	screenRectangleNode = postProcessor.AddUINODE(S_DEVICE, postProcessor.getRoot(), SVec2(0, 0), SVec2(1, 1));

	//hexer initialization
	
	Procedural::Geometry hex;
	hex.GenHexaprism(hexRadius, 10.f);
	hexModel.meshes.push_back(Mesh(hex, S_DEVICE));
	hexer.init(hexRadius);
	isFirst = true;
	SVec3 initPlayerPos(hexer._points[0].x, hexer._points[0].y + 40.f, hexer._points[0].z);
	S_RANDY.setCameraMatrix(SMatrix::CreateTranslation(initPlayerPos));

	skybox.LoadModel(S_DEVICE, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(S_DEVICE, "../Textures/day.dds");

	will.LoadModel(S_DEVICE, "../Models/ball.fbx");
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

	terrain.setTextureData(S_DEVICE, 10, 10, { "../Textures/LavaCracks/diffuse.png", "../Textures/LavaCracks/normal.png" });
	terrain.SetUp(S_DEVICE);
	
	Procedural::Terrain island = Procedural::Terrain(128, 128, SVec3(8, 300, 8));
	island.Mesa(SVec2(256), 32, 64, 64);
	island.Mesa(SVec2(256), 16, 16, -128);
	island.setTextureData(S_DEVICE, 10, 10, { "../Textures/Lava/diffuse.jpg", "../Textures/Lava/normal.jpg" });
	island.CalculateNormals();
	island.CalculateTexCoords();
	islandModel = Model(island, S_DEVICE);


	lavaSheet = Procedural::Terrain(2, 2, SVec3(1024, 1, 1024));
	lavaSheet.setOffset(0, 32, 0);
	lavaSheet.CalculateTexCoords();
	lavaSheet.CalculateNormals();

	lavaSheetModel = Model(lavaSheet, S_DEVICE);

	//textures
	hexDiffuseMap.LoadFromFile("../Textures/Crymetal/diffuse.jpg");
	hexDiffuseMap.Setup(S_DEVICE);
	hexNormalMap.LoadFromFile("../Textures/Crymetal/normal.jpg");
	hexNormalMap.Setup(S_DEVICE);

	Procedural::LSystem linden;
	linden.reseed("F");
	linden.addRule('F', "FF+[+*F-F-/F]*-[-F/+F+*F]/");
	linden.rewrite(4);
	float liangle = PI * 0.138888f;		//liangle = PI * .5f;
	tree = linden.genModel(S_DEVICE, 12.f, 3.f, .77f, .77f, liangle, liangle);
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
		hexCluster.meshes.emplace_back(h, S_DEVICE);

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
	{
		hexer._platforms.push_back(Platform(potentialPlatformPos, &hexModel, &_sys._renderer._shMan.normalMapper));
		if (isFirst)
		{
			isFirst = false;
			S_RANDY._cam.SetCameraMatrix((hexer._platforms.back().actor.transform));
			_sys._renderer._cam.Translate(SVec3(0, 20, 0));
		}
	}
	
	if (!_sys._defController.isFlying())
	{
		SVec3 oldPos = _sys._renderer._cam.GetCameraMatrix().Translation();
		float newHeight = terrain.getHeightAtPosition(rc.cam->GetPosition());
		if (rc.cam->GetPosition().y < newHeight)
			rc.cam->SetTranslation(SVec3(oldPos.x, newHeight, oldPos.z));
	}

	win(rc.cam->GetPosition());

	if (rc.cam->GetPosition().y <= lavaSheetModel.transform.Translation().y)
	{
		S_RANDY.setCameraMatrix(SMatrix::CreateTranslation(hexer._points[0]));
		hexer.init(hexRadius);
		isFirst = true;
	}
		
}



void FireLevel::draw(const RenderContext& rc)
{
	sceneTex.SetRenderTarget(S_CONTEXT);

	terrain.Draw(S_CONTEXT, S_SHADY.terrainNormals, *rc.cam, pointLight, rc.elapsed);

		
	S_SHADY.terrainNormals.SetShaderParameters(S_CONTEXT, SMatrix::CreateTranslation(hexer._points.back().x - 256.f, 0, hexer._points.back().z - 256.f), *rc.cam, pointLight, rc.dTime);
	islandModel.Draw(S_CONTEXT, S_SHADY.terrainNormals);
	
	for (Platform p : hexer._platforms)
	{
		if (!p.active) continue;

		hexModel.transform = p.actor.transform;
		S_SHADY.normalMapper.SetShaderParameters(S_CONTEXT, hexModel, *rc.cam, pointLight, rc.dTime, hexDiffuseMap, hexNormalMap);
		hexModel.Draw(S_CONTEXT, S_SHADY.normalMapper);
	}

	S_SHADY.normalMapper.SetShaderParameters(S_CONTEXT, tree, *rc.cam, pointLight, rc.dTime, tree.textures_loaded[0], tree.textures_loaded[1]);
	tree.Draw(S_CONTEXT, S_SHADY.normalMapper);

	S_RANDY.renderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	//transparent items
	rc.d3d->TurnOnAlphaBlending();
	S_SHADY.shVolumLava.SetShaderParameters(S_CONTEXT, lavaSheetModel, *rc.cam, rc.elapsed);
	lavaSheetModel.Draw(S_CONTEXT, S_SHADY.shVolumLava);

	S_SHADY.shVolumFire.SetShaderParameters(S_CONTEXT, will, *rc.cam, rc.elapsed);
	will.Draw(S_CONTEXT, S_SHADY.shVolumFire);
	rc.d3d->TurnOffAlphaBlending();



	//brightnessMask
	brightnessMask.SetRenderTarget(S_CONTEXT);
	postProcessor.draw(S_CONTEXT, S_SHADY.brightnessMasker, sceneTex.srv);

	//blurring horizontally
	blurredTex1.SetRenderTarget(S_CONTEXT);
	postProcessor.draw(S_CONTEXT, S_SHADY.blurHor, brightnessMask.srv);

	//blurring vertically
	blurredTex2.SetRenderTarget(S_CONTEXT);
	postProcessor.draw(S_CONTEXT, S_SHADY.blurVer, blurredTex1.srv);

	for (int i = 0; i < 3; ++i)
	{
		//blurring horizontally
		blurredTex1.SetRenderTarget(S_CONTEXT);
		postProcessor.draw(S_CONTEXT, S_SHADY.blurHor, blurredTex2.srv);

		//blurring vertically
		blurredTex2.SetRenderTarget(S_CONTEXT);
		postProcessor.draw(S_CONTEXT, S_SHADY.blurVer, blurredTex1.srv);
	}
	
	//final scene rendering - the screen quad
	rc.d3d->SetBackBufferRenderTarget();						//set default screen buffer as output target

	postProcessor.draw(S_CONTEXT, S_SHADY.bloom, sceneTex.srv, blurredTex2.srv);
	
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
		
		for (Renderable r : platform.actor.renderables)
			c.hulls.push_back(S_COLLISION.genBoxHull(r.mesh, SMatrix::Identity));

		for (Hull* h : c.hulls)
		{
			SVec4 parentPos = c.actParent->transform.Translation();
			SVec3 childPos = SVec3(parentPos.x, parentPos.y, parentPos.z);
			h->setPosition(childPos);
		}

		S_COLLISION.addToGrid(&c);

		_levelColliders.push_back(c);
	}
}



void FireLevel::resetCollision()
{
	for (Collider& collider : _levelColliders)
	{
		S_COLLISION.removeFromGrid(collider);
		collider.ReleaseMemory();
	}

	_levelColliders.clear();
}



bool FireLevel::processInput(float dTime)
{
	ProcessSpecialInput(dTime);

	if (_sys._inputManager.isKeyDown((short)'M'))
	{
		return true;
	}

	return false;
}