#include "WaterLevel.h"


void WaterLevel::init(Systems& sys)
{
	_sys._colEngine.registerController(_sys._controller);

	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

	fence.LoadModel(device, "../Models/WaterFence.obj");
	
	lillyModel.LoadModel(device, "../Models/Lilly/pizzaBox.obj");	//Lilly.obj
	Math::Scale(lillyModel.transform, SVec3(33.333333f));
	colModel.LoadModel(device, "../Models/Lilly/pizzaBox.obj");

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
	waterTerrain.setTextureData(device, 1, 1, { "../Textures/LavaIntense/diffuse.jpg", "../Textures/LavaIntense/normal.jpg" });
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

	_lillies.init(20.f, .666666f, SVec2(40, 200), SVec3(0, waterTerrain.getOffset().y - .2f, 0));

	setUpCollision();

	/*
	linden.reseed("F");
	linden.addRule('F', "FF+[+F-F-F]*-[-F+F+F]/"); //"[-F]*F[+F][/F]"	//"F[+F]F[-F]+F" for planar		//"FF+[+F-F-F]*-[-F+F+F]/"
	//linden.addRule('F', "F*[+F-F-F]-[-F+F+F]/");
	linden.rewrite(4);
	float liangle = 24.f * PI / 180.f;		//liangle = PI * .5f;

	treeModel = linden.genModel(device, 3.f, .4f, .7f, .7f, liangle, liangle);
	Math::Scale(treeModel.transform, SVec3(2));
	Math::RotateMatByMat(treeModel.transform, SMatrix::CreateRotationY(PI));
	Math::SetTranslation(treeModel.transform, SVec3(-6, 143, 56));	// SVec3(-10, 126, 20)
	*/
}



void WaterLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers(rc.d3d->clearColour);
	
	_lillies.update(rc.dTime);
	updateCollision();
	updateCam(rc.dTime);
	ProcessSpecialInput(rc.dTime);
	
	cubeMapper.UpdateCams(modBall.transform.Translation());
	for (int i = 0; i < 6; i++)
	{
		updateReflectionRefraction(rc, cubeMapper.getCameraAtIndex(i));
		cubeMapper.Advance(_sys._deviceContext, i);

		//lotus
		rc.shMan->light.SetShaderParameters(dc, lotus, cubeMapper.getCameraAtIndex(i), pointLight, rc.dTime);
		lotus.Draw(dc, rc.shMan->light);
		rc.shMan->light.ReleaseShaderParameters(dc);
		
		//water
		rc.shMan->water.SetShaderParameters(dc, waterSheet, cubeMapper.getCameraAtIndex(i), pointLight, rc.elapsed, waterNormalMap.srv, reflectionMap.srv, refractionMap.srv);
		waterSheet.Draw(dc, rc.shMan->water);
		rc.shMan->water.ReleaseShaderParameters(dc);

		_sys._renderer.RenderSkybox(cubeMapper.getCameraAtIndex(i), skybox, skyboxCubeMapper);
	}
	updateReflectionRefraction(rc, *rc.cam);

	///scene
	_sys._renderer.RevertRenderTarget();

	//lotus
	rc.shMan->light.SetShaderParameters(dc, lotus, *rc.cam, pointLight, rc.dTime);
	lotus.Draw(dc, rc.shMan->light);
	rc.shMan->light.ReleaseShaderParameters(dc);

	//rc.shMan->terrainNormals.SetShaderParameters(dc, treeModel.transform, *rc.cam, pointLight, rc.elapsed);
	//treeModel.Draw(dc, rc.shMan->terrainNormals);
	//rc.shMan->terrainNormals.ReleaseShaderParameters(dc);

	//water
	rc.d3d->TurnOnAlphaBlending();
	
	rc.shMan->water.SetShaderParameters(dc, waterSheet, *rc.cam, pointLight, rc.elapsed, waterNormalMap.srv, reflectionMap.srv, refractionMap.srv);
	waterSheet.Draw(dc, rc.shMan->water);
	rc.shMan->water.ReleaseShaderParameters(dc);

	//_lillies.draw(rc, lillyModel, pointLight, true);
	rc.d3d->TurnOffAlphaBlending();

	for (int i = 0; i < _levelColliders.size(); ++i)
	{
		colModel.transform = _levelColliders[i].actParent->transform;
		rc.shMan->shVolumLava.SetShaderParameters(dc, colModel, *rc.cam, rc.dTime);
		colModel.Draw(dc, rc.shMan->shVolumLava);
		rc.shMan->light.ReleaseShaderParameters(_sys._deviceContext);
	}

	//reflection sphere
	rc.shMan->cubeMapShader.SetShaderParameters(dc, modBall, *rc.cam, pointLight, rc.dTime, cubeMapper.cm_srv);
	modBall.Draw(dc, rc.shMan->cubeMapShader);
	rc.shMan->cubeMapShader.ReleaseShaderParameters(dc);


	//SKYBOX
	_sys._renderer.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	/*
	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumWater.SetShaderParameters(dc, will, *rc.cam, rc.elapsed);
	will.Draw(dc, rc.shMan->shVolumWater);
	rc.d3d->TurnOffAlphaBlending();
	*/

	rc.d3d->EndScene();
}



void WaterLevel::updateReflectionRefraction(const RenderContext& rc, const Camera& cam)
{
	///reflection
	reflectionMap.SetRenderTarget(_sys._deviceContext);
	_sys._renderer._shMan.clipper.SetClipper(_sys._deviceContext, SVec4(0, 1, 0, -waterSheet.transform.Translation().y));
	Camera reflectionCam = Camera(cam.GetCameraMatrix() * waterReflectionMatrix, cam.GetProjectionMatrix());

	rc.shMan->clipper.SetShaderParameters(dc, lotus, reflectionCam, pointLight, rc.dTime);
	lotus.Draw(dc, rc.shMan->clipper);
	rc.shMan->clipper.ReleaseShaderParameters(dc);

	///refraction
	refractionMap.SetRenderTarget(_sys._deviceContext);
	_sys._renderer._shMan.clipper.SetClipper(_sys._deviceContext, SVec4(0, -1, 0, waterSheet.transform.Translation().y));

	rc.shMan->clipper.SetShaderParameters(dc, lotus, cam, pointLight, rc.dTime);
	lotus.Draw(dc, rc.shMan->clipper);
	rc.shMan->clipper.ReleaseShaderParameters(dc);
}



void WaterLevel::setUpCollision()
{
	_levelColliders.reserve(100);

	for (Mesh& m : lillyModel.meshes)
		for (Vert3D& v : m.vertices)
			v.pos = SVec3::Transform(v.pos, lillyModel.transform);

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