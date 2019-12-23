#include "WaterLevel.h"


void WaterLevel::init(Systems& sys)
{
	_sys._colEngine.registerController(_sys._controller);

	skybox.LoadModel(S_DEVICE, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(S_DEVICE, "../Textures/night.dds");

	cubeMapper.Init(S_DEVICE);

	modBall.LoadModel(S_DEVICE, "../Models/ball.fbx");
	Math::Scale(modBall.transform, SVec3(20));
	Math::Translate(modBall.transform, SVec3(0, 164, 0));

	lotus.LoadModel(S_DEVICE, "../Models/crownless_wider.obj");
	Math::Scale(lotus.transform, SVec3(150));

	lotusTex.LoadFromFile("../Textures/Lotus/diffuse.jpg");
	lotusTex.Setup(S_DEVICE);
	for (auto& m : lotus.meshes)
		m.textures.push_back(lotusTex);

	waterTerrain = Procedural::Terrain(256, 256, SVec3(2, 1, 2));
	//waterTerrain.setTextureData(device, 1, 1, { "../Textures/LavaIntense/diffuse.jpg", "../Textures/LavaIntense/normal.jpg" });
	waterTerrain.setOffset(-256, 96, -256);
	waterTerrain.CalculateTexCoords();
	waterTerrain.CalculateNormals();

	waterSheet = Model(waterTerrain, _sys._device);

	waterNormalMap.LoadFromFile("../Textures/Water/normal.jpg");
	waterNormalMap.Setup(S_DEVICE);
	
	reflectionMap.Init(S_DEVICE, 1600, 900);	//@make false after testing for both of them!
	refractionMap.Init(S_DEVICE, 1600, 900);
	waterReflectionMatrix = SMatrix::CreateReflection(SPlane(SVec3(0, 1, 0), -waterSheet.transform.Translation().y));

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(0, 500.f, 0, 1.0f));

	_lillies.init(33.333f, .1667f, SVec2(33.333, 200.f), SVec3(0, waterTerrain.getOffset().y - .2f, 0));
	lillyModel.LoadModel(S_DEVICE, "../Models/Lilly/Lilly.obj");

	
	throne.LoadModel(S_DEVICE, "../Models/FlowerThrone/flowerThrone.fbx");
	Math::Scale(throne.transform, SVec3(.33));
	Math::RotateMatByMat(throne.transform, SMatrix::CreateFromAxisAngle(SVec3(1, 0, 0), PI * 0.5f));
	Math::RotateMatByMat(throne.transform, SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), -PI * 0.5f));
	throne.transform *= SMatrix::CreateTranslation(SVec3(0, waterTerrain.getOffset().y - 20.f, 200));
	goal = throne.transform.Translation();

	will.LoadModel(S_DEVICE, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(10.f));
	will.transform *= SMatrix::CreateTranslation(SVec3(goal.x, goal.y + 50, goal.z));

	_startingTransform = SMatrix::CreateTranslation(SVec3(0, waterTerrain.getOffset().y + 10.f, -200));
	S_RANDY.setCameraMatrix(_startingTransform);
	
	plat.LoadModel(S_DEVICE, "../Models/leaf.fbx");
	plat.transform = SMatrix::CreateScale(10);
	plat.transform *= SMatrix::CreateRotationY(-PI * 0.5f);
	plat.transform *= _startingTransform;
	S_COLLISION.registerModel(plat, BVT_AABB);
	S_COLLISION.registerController(_sys._controller);

	SMatrix petalScale = SMatrix::CreateScale(SVec3(5));

	lillyPetalModel.LoadModel(S_DEVICE, "../Models/lotus_petal_upright.obj");
	for (Vert3D& v : lillyPetalModel.meshes[0].vertices)
		v.pos = SVec3::Transform(v.pos, petalScale);

	Texture lillyPetalTex(S_DEVICE, "../Textures/Lotus/diffuse.jpg");
	lillyPetalModel.meshes[0].textures.push_back(lillyPetalTex);

	fence.LoadModel(S_DEVICE, "../Models/barrier.obj");
	Math::Scale(fence.transform, SVec3(2));
	fence.transform *= SMatrix::CreateTranslation(SVec3(132, waterTerrain.getOffset().y, 22));
	fence.meshes[0].textures.push_back(lillyPetalTex);

	linden.reseed("F");
	linden.addRule('F', "FDSL");	// f just controls iterations, d is a displacement, s is a stalk, l becomes a level of petals
	linden.rewrite(4);
	linden.removeRule('F', "FDSL");
	linden.addRule('L', "[+P*P*P*P*P*P]");
	linden.rewrite(1);

	float petalToPetalAngle = 60.f * PI / 180.f;
	float stalkToPetalAngle = 90.f * PI / 180.f;

	flowerModel = linden.genFlower(S_DEVICE, &lillyPetalModel, 1, .33, .7f, petalToPetalAngle, stalkToPetalAngle);
	Math::Scale(flowerModel.transform, SVec3(2));
	Math::RotateMatByMat(flowerModel.transform, SMatrix::CreateRotationY(PI));
	Math::SetTranslation(flowerModel.transform, SVec3(0, 90, 0));	// SVec3(-10, 126, 20)
}



void WaterLevel::update(const RenderContext& rc)
{
	win(rc.cam->GetPosition());

	_lillies.update(rc.dTime);
	fakeCollision();
	updateCam(rc.dTime);

	//death resets position to the beginning
	if (rc.cam->GetPosition().y < waterTerrain.getOffset().y - 5)
	{
		rc.cam->SetCameraMatrix(_startingTransform);
	}

	SVec3 fenceToCam = rc.cam->GetPosition() - fence.transform.Translation();
	if (fenceToCam.Length() < 33.f)
	{
		rc.cam->SetTranslation(fence.transform.Translation() + Math::getNormalizedVec3(fenceToCam) * 33.f);
	}


	ProcessSpecialInput(rc.dTime);
}



void WaterLevel::draw(const RenderContext& rc)
{
	rc.d3d->SetBackBufferRenderTarget();
	
#pragma region reflectionRendering
	cubeMapper.UpdateCams(modBall.transform.Translation());
	for (int i = 0; i < 6; i++)
	{
		updateReflectionRefraction(rc, cubeMapper.getCameraAtIndex(i)); 
		cubeMapper.Advance(_sys._deviceContext, i);

		//water
		rc.shMan->water.SetShaderParameters(S_CONTEXT, waterSheet, cubeMapper.getCameraAtIndex(i), pointLight, rc.elapsed, waterNormalMap.srv, reflectionMap.srv, refractionMap.srv);
		waterSheet.Draw(S_CONTEXT, rc.shMan->water);
		rc.shMan->water.ReleaseShaderParameters(S_CONTEXT);

		RenderContext rcTemp = rc;
		rcTemp.cam = &cubeMapper.getCameraAtIndex(i);
		_lillies.draw(rcTemp, lillyModel, pointLight, true);

		//lotus
		rc.shMan->light.SetShaderParameters(S_CONTEXT, lotus.transform, cubeMapper.getCameraAtIndex(i), pointLight, rc.dTime);
		lotus.Draw(S_CONTEXT, rc.shMan->light);
		rc.shMan->light.ReleaseShaderParameters(S_CONTEXT);


		S_RANDY.renderSkybox(cubeMapper.getCameraAtIndex(i), skybox, skyboxCubeMapper);
	}
	updateReflectionRefraction(rc, *rc.cam);
#pragma endregion reflectionRendering


	///scene
	S_RANDY.setDefaultRenderTarget();

	//lsystems
	rc.shMan->light.SetShaderParameters(S_CONTEXT, flowerModel.transform, *rc.cam, pointLight, rc.elapsed);
	flowerModel.Draw(S_CONTEXT, rc.shMan->light);
	rc.shMan->light.ReleaseShaderParameters(S_CONTEXT);

	//lotus
	rc.shMan->light.SetShaderParameters(S_CONTEXT, lotus.transform, *rc.cam, pointLight, rc.dTime);
	lotus.Draw(S_CONTEXT, rc.shMan->light);
	rc.shMan->light.ReleaseShaderParameters(S_CONTEXT);

	//platform
	rc.shMan->light.SetShaderParameters(S_CONTEXT, plat.transform, *rc.cam, pointLight, rc.dTime);
	plat.Draw(S_CONTEXT, rc.shMan->light);
	rc.shMan->light.ReleaseShaderParameters(S_CONTEXT);

	//water
	rc.d3d->TurnOnAlphaBlending();
	S_SHADY.water.SetShaderParameters(S_CONTEXT, waterSheet, *rc.cam, pointLight, rc.elapsed, waterNormalMap.srv, reflectionMap.srv, refractionMap.srv);
	waterSheet.Draw(S_CONTEXT, rc.shMan->water);
	S_SHADY.water.ReleaseShaderParameters(S_CONTEXT);

	_lillies.draw(rc, lillyModel, pointLight, false);
	rc.d3d->TurnOffAlphaBlending();

	S_SHADY.terrainNormals.SetShaderParameters(S_CONTEXT, throne.transform, *rc.cam, pointLight, rc.dTime);
	throne.Draw(S_CONTEXT, S_SHADY.terrainNormals);

	S_SHADY.light.SetShaderParameters(S_CONTEXT, fence.transform, *rc.cam, pointLight, rc.dTime);
	fence.Draw(S_CONTEXT, S_SHADY.terrainNormals);
	
	//reflection sphere
	rc.shMan->cubeMapShader.SetShaderParameters(S_CONTEXT, modBall, *rc.cam, pointLight, rc.dTime, cubeMapper.cm_srv);
	modBall.Draw(S_CONTEXT, rc.shMan->cubeMapShader);
	rc.shMan->cubeMapShader.ReleaseShaderParameters(S_CONTEXT);


	//SKYBOX
	S_RANDY.renderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	//water fairy
	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumWater.SetShaderParameters(S_CONTEXT, will, *rc.cam, rc.elapsed);
	will.Draw(S_CONTEXT, rc.shMan->shVolumWater);
	rc.d3d->TurnOffAlphaBlending();


	rc.d3d->EndScene();
}



void WaterLevel::updateReflectionRefraction(const RenderContext& rc, const Camera& cam)
{
	///reflection
	reflectionMap.SetRenderTarget(_sys._deviceContext);
	S_RANDY._shMan.clipper.SetClipper(_sys._deviceContext, SVec4(0, 1, 0, -waterSheet.transform.Translation().y));
	Camera reflectionCam = Camera(cam.GetCameraMatrix() * waterReflectionMatrix, cam.GetProjectionMatrix());

	rc.shMan->clipper.SetShaderParameters(S_CONTEXT, lotus, reflectionCam, pointLight, rc.dTime);
	lotus.Draw(S_CONTEXT, rc.shMan->clipper);
	rc.shMan->clipper.ReleaseShaderParameters(S_CONTEXT);

	///refraction
	refractionMap.SetRenderTarget(_sys._deviceContext);
	S_RANDY._shMan.clipper.SetClipper(_sys._deviceContext, SVec4(0, -1, 0, waterSheet.transform.Translation().y));

	rc.shMan->clipper.SetShaderParameters(S_CONTEXT, lotus, cam, pointLight, rc.dTime);
	lotus.Draw(S_CONTEXT, rc.shMan->clipper);
	rc.shMan->clipper.ReleaseShaderParameters(S_CONTEXT);
}



void WaterLevel::setUpCollision()
{
	_levelColliders.reserve(100);

	//for (Mesh& m : lillyModel.meshes)
		//for (Vert3D& v : m.vertices)
			//v.pos = SVec3::Transform(v.pos, lillyModel.transform);

	for (Ring& ring : _lillies._lillyRings)
	{
		for (Lilly& lil : ring._lillies)
		{
			Collider c;
			c.BVT = BVT_AABB;
			c.actParent = &lil.act;
			c.dynamic = true;

			c.hulls.push_back(_sys._colEngine.genBoxHull(&lillyModel.meshes[0], lil.act.transform, &c));
			c.hulls.push_back(_sys._colEngine.genBoxHull(&lillyModel.meshes[1], lil.act.transform, &c));

			for (Hull* h : c.hulls)
			{
				SVec3 p = c.actParent->transform.Translation();
				h->setPosition(p);
			}

			_levelColliders.push_back(c);
			lil.act.collider = &_levelColliders.back();
			_sys._colEngine.addToGrid(&_levelColliders.back());
		}
	}
}


//updates collider transforms of all actors, as well as adding them to hulls
void WaterLevel::updateCollision()
{
	for (Ring& ring : _lillies._lillyRings)
		for (Lilly& l : ring._lillies)
			for (Hull* h : l.act.collider->hulls)
				h->setPosition(l.act.transform.Translation());
}



void WaterLevel::fakeCollision()
{
	bool wasOnLilly = parentLilly != nullptr;
	bool onLilly = false;

	for (Ring& ring : _lillies._lillyRings)
		for (Lilly& l : ring._lillies)
			if (SVec3::DistanceSquared(S_RANDY._cam.GetCameraMatrix().Translation(), l.act.transform.Translation()) < 33.333 * 33.333
				&& !_sys._controller.isFlying()
				&& l.real)
			{
				onLilly = true;
				if (!wasOnLilly)
					oldTranslation = l.act.transform.Translation();
				parentLilly = &l;
			}
				

	if (onLilly)
	{
		SMatrix old = S_RANDY._cam.GetCameraMatrix();
		SVec3 oldTranslation = old.Translation();
		oldTranslation.y = waterSheet.transform.Translation().y + 5.f;
		Math::SetTranslation(old, oldTranslation);
		S_RANDY._cam.SetCameraMatrix(old);
	}
	else
	{
		parentLilly = nullptr;
	}
	
	if (parentLilly != nullptr)
	{
		SMatrix old = S_RANDY._cam.GetCameraMatrix();
		SVec3 delta = parentLilly->act.transform.Translation() - oldTranslation;
		Math::Translate(old, delta);
		S_RANDY._cam.SetCameraMatrix(old);
		oldTranslation = parentLilly->act.transform.Translation();
	}
}


//linden.addRule('F', "FF+[+F-F-F]*-[-F+F+F]/");		//float liangle = 24.f * PI / 180.f;
//linden.addRule('F', "F*[+F-F-F]-[-F+F+F]/");
//linden.addRule('F', "F[+F]F[-F]F");
//flowerModel = linden.genModel(device, 3.f, .4f, .7f, .7f, liangle, liangle);