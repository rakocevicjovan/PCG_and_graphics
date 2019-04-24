#include "WaterLevel.h"


void WaterLevel::init(Systems& sys)
{
	_sys._colEngine.registerController(_sys._controller);

	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

	cubeMapper.Init(device);

	will.LoadModel(device, "../Models/ball.fbx");

	modBall.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(modBall.transform, SVec3(20));
	Math::Translate(modBall.transform, SVec3(0, 164, 0));

	lotus.LoadModel(device, "../Models/crownless_wider.obj");
	Math::Scale(lotus.transform, SVec3(150));

	lotusTex.LoadFromFile("../Textures/Lotus/diffuse.jpg");
	lotusTex.Setup(device);
	for (auto& m : lotus.meshes)
		m.textures.push_back(lotusTex);

	waterTerrain = Procedural::Terrain(256, 256, SVec3(2, 1, 2));
	//waterTerrain.setTextureData(device, 1, 1, { "../Textures/LavaIntense/diffuse.jpg", "../Textures/LavaIntense/normal.jpg" });
	waterTerrain.setOffset(-256, 96, -256);
	waterTerrain.CalculateTexCoords();
	waterTerrain.CalculateNormals();

	waterSheet = Model(waterTerrain, _sys._device);

	waterNormalMap.LoadFromFile("../Textures/Water/normal.jpg");
	waterNormalMap.Setup(device);
	
	reflectionMap.Init(device, 1600, 900);	//@make false after testing for both of them!
	refractionMap.Init(device, 1600, 900);
	waterReflectionMatrix = SMatrix::CreateReflection(SPlane(SVec3(0, 1, 0), -waterSheet.transform.Translation().y));

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(0, 500.f, 0, 1.0f));

	_lillies.init(33.333f, .1667f, SVec2(33.333, 200.f), SVec3(0, waterTerrain.getOffset().y - .2f, 0));
	lillyModel.LoadModel(device, "../Models/Lilly/Lilly.obj");
	
	SMatrix petalScale = SMatrix::CreateScale(SVec3(5));

	lillyPetalModel.LoadModel(device, "../Models/lotus_petal_upright.obj");
	for (Vert3D& v : lillyPetalModel.meshes[0].vertices)
		v.pos = SVec3::Transform(v.pos, petalScale);
	
	Texture lillyPetalTex(device, "../Textures/Lotus/diffuse.jpg");
	lillyPetalModel.meshes[0].textures.push_back(lillyPetalTex);

	linden.reseed("F");
	linden.addRule('F', "FDSL");	// f just controls iterations, d is a displacement, s is a stalk, l becomes a level of petals
	linden.rewrite(4);
	linden.removeRule('F', "FDSL");
	linden.addRule('L', "[+P*P*P*P*P*P]");
	linden.rewrite(1);

	float petalToPetalAngle = 60.f * PI / 180.f;
	float stalkToPetalAngle = 90.f * PI / 180.f;

	treeModel = linden.genFlower(device, &lillyPetalModel, 1, .33, .7f, petalToPetalAngle, stalkToPetalAngle);
	Math::Scale(treeModel.transform, SVec3(2));
	Math::RotateMatByMat(treeModel.transform, SMatrix::CreateRotationY(PI));
	Math::SetTranslation(treeModel.transform, SVec3(0, 90, 0));	// SVec3(-10, 126, 20)

}



void WaterLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers(rc.d3d->clearColour);
	
	_lillies.update(rc.dTime);
	fakeCollision();
	//updateCollision();
	updateCam(rc.dTime);
		
	ProcessSpecialInput(rc.dTime);
	
#pragma region reflectionRendering
	cubeMapper.UpdateCams(modBall.transform.Translation());
	for (int i = 0; i < 6; i++)
	{
		updateReflectionRefraction(rc, cubeMapper.getCameraAtIndex(i)); 
		cubeMapper.Advance(_sys._deviceContext, i);

		//water
		rc.shMan->water.SetShaderParameters(context, waterSheet, cubeMapper.getCameraAtIndex(i), pointLight, rc.elapsed, waterNormalMap.srv, reflectionMap.srv, refractionMap.srv);
		waterSheet.Draw(context, rc.shMan->water);
		rc.shMan->water.ReleaseShaderParameters(context);


		RenderContext rcTemp = rc;
		rcTemp.cam = &cubeMapper.getCameraAtIndex(i);
		_lillies.draw(rcTemp, lillyModel, pointLight, true);

		//lotus
		rc.shMan->light.SetShaderParameters(context, lotus.transform, cubeMapper.getCameraAtIndex(i), pointLight, rc.dTime);
		lotus.Draw(context, rc.shMan->light);
		rc.shMan->light.ReleaseShaderParameters(context);


		randy.RenderSkybox(cubeMapper.getCameraAtIndex(i), skybox, skyboxCubeMapper);
	}
	updateReflectionRefraction(rc, *rc.cam);
#pragma endregion reflectionRendering


	///scene
	randy.RevertRenderTarget();

	//lotus
	rc.shMan->light.SetShaderParameters(context, lotus.transform, *rc.cam, pointLight, rc.dTime);
	lotus.Draw(context, rc.shMan->light);
	rc.shMan->light.ReleaseShaderParameters(context);

	//lsystems
	rc.shMan->terrainNormals.SetShaderParameters(context, treeModel.transform, *rc.cam, pointLight, rc.elapsed);
	treeModel.Draw(context, rc.shMan->terrainNormals);
	rc.shMan->terrainNormals.ReleaseShaderParameters(context);

	//water
	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->water.SetShaderParameters(context, waterSheet, *rc.cam, pointLight, rc.elapsed, waterNormalMap.srv, reflectionMap.srv, refractionMap.srv);
	waterSheet.Draw(context, rc.shMan->water);
	rc.shMan->water.ReleaseShaderParameters(context);

	_lillies.draw(rc, lillyModel, pointLight, false);
	rc.d3d->TurnOffAlphaBlending();

	//reflection sphere
	rc.shMan->cubeMapShader.SetShaderParameters(context, modBall, *rc.cam, pointLight, rc.dTime, cubeMapper.cm_srv);
	modBall.Draw(context, rc.shMan->cubeMapShader);
	rc.shMan->cubeMapShader.ReleaseShaderParameters(context);


	//SKYBOX
	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	/*
	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumWater.SetShaderParameters(context, will, *rc.cam, rc.elapsed);
	will.Draw(context, rc.shMan->shVolumWater);
	rc.d3d->TurnOffAlphaBlending();
	*/

	rc.d3d->EndScene();
}



void WaterLevel::updateReflectionRefraction(const RenderContext& rc, const Camera& cam)
{
	///reflection
	reflectionMap.SetRenderTarget(_sys._deviceContext);
	randy._shMan.clipper.SetClipper(_sys._deviceContext, SVec4(0, 1, 0, -waterSheet.transform.Translation().y));
	Camera reflectionCam = Camera(cam.GetCameraMatrix() * waterReflectionMatrix, cam.GetProjectionMatrix());

	rc.shMan->clipper.SetShaderParameters(context, lotus, reflectionCam, pointLight, rc.dTime);
	lotus.Draw(context, rc.shMan->clipper);
	rc.shMan->clipper.ReleaseShaderParameters(context);

	///refraction
	refractionMap.SetRenderTarget(_sys._deviceContext);
	randy._shMan.clipper.SetClipper(_sys._deviceContext, SVec4(0, -1, 0, waterSheet.transform.Translation().y));

	rc.shMan->clipper.SetShaderParameters(context, lotus, cam, pointLight, rc.dTime);
	lotus.Draw(context, rc.shMan->clipper);
	rc.shMan->clipper.ReleaseShaderParameters(context);
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

			c.hulls.push_back(_sys._colEngine.genBoxHull(&lillyModel.meshes[0], &c));
			c.hulls.push_back(_sys._colEngine.genBoxHull(&lillyModel.meshes[1], &c));

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
	{
		for (Lilly& l : ring._lillies)
		{
			for (Hull* h : l.act.collider->hulls)
			{
				h->setPosition(l.act.transform.Translation());
			}
		}
	}
}



void WaterLevel::fakeCollision()
{
	bool wasOnLilly = parentLilly != nullptr;
	bool onLilly = false;

	for (Ring& ring : _lillies._lillyRings)
		for (Lilly& l : ring._lillies)
			if (SVec3::DistanceSquared(randy._cam.GetCameraMatrix().Translation(), l.act.transform.Translation()) < 33.333 * 33.333
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
		SMatrix old = randy._cam.GetCameraMatrix();
		SVec3 oldTranslation = old.Translation();
		oldTranslation.y = waterSheet.transform.Translation().y + 5.f;
		Math::SetTranslation(old, oldTranslation);
		randy._cam.SetCameraMatrix(old);
	}
	else
	{
		parentLilly = nullptr;
	}
	
	if (parentLilly != nullptr)
	{
		SMatrix old = randy._cam.GetCameraMatrix();
		SVec3 delta = parentLilly->act.transform.Translation() - oldTranslation;
		Math::Translate(old, delta);
		randy._cam.SetCameraMatrix(old);
		oldTranslation = parentLilly->act.transform.Translation();
	}
}


//linden.addRule('F', "FF+[+F-F-F]*-[-F+F+F]/");		//float liangle = 24.f * PI / 180.f;
//linden.addRule('F', "F*[+F-F-F]-[-F+F+F]/");
//linden.addRule('F', "F[+F]F[-F]F");
//treeModel = linden.genModel(device, 3.f, .4f, .7f, .7f, liangle, liangle);