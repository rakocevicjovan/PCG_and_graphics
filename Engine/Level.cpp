#include "Level.h"
#include "Controller.h"
#include <d3d11.h>

Level::Level(Systems& sys) : _sys(&sys) {}

#define device _sys->_device
#define dc _sys->_deviceContext

void EarthLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(5.f));
	Math::Translate(will.transform, SVec3(2, 35, 60));

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(333.f, 666.f, 999.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)

	postProcessTexture.Init(device, ostW, ostH);

	mazeDiffuseMap.LoadFromFile("../Textures/Rock/diffuse.jpg");
	mazeDiffuseMap.Setup(device);
	mazeNormalMap.LoadFromFile("../Textures/Rock/normal.jpg");
	mazeNormalMap.Setup(device);

	pSys.init(device, 100, SVec3(0, 0, 100), "../Models/ball.fbx");

	particleUpdFunc1 = [this](ParticleUpdateData* pud) -> void
	{
		for (int i = 0; i < pSys._particles.size(); ++i)
		{
			pSys._particles[i]->age += pud->dTime * 0.1f;
			SVec3 translation(pud->windDirection * pud->windVelocity);	// 
			translation.x *= sin(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			translation.y *= cos(pSys._particles[i]->age  * ((float)pSys._particles.size() - (float)i));
			translation.z *= cos(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			Math::SetTranslation(pSys._particles[i]->transform, translation * (float)i * 0.33f);
		}
	};

	particleUpdFunc2 = [this](ParticleUpdateData* pud) -> void
	{
		for (int i = 0; i < pSys._particles.size(); ++i)
		{
			pSys._particles[i]->age += pud->dTime * 0.1f;
			SVec3 translation(pud->windDirection * pud->windVelocity);
			translation.x *= sin(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			translation.y *= cos(pSys._particles[i]->age  * ((float)pSys._particles.size() - (float)i));
			translation.z *= cos(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			Math::SetTranslation(pSys._particles[i]->transform, translation * (float)i * -0.33f);
		}
	};

	pSys.setUpdateFunction(particleUpdFunc1);
	//shader needs to be set for the particle system somehow as well...

	maze.Init(10, 10, 32.f);
	maze.CreateModel(device);
}



void EarthLevel::procGen()
{
	if (isTerGenerated) return;

	proceduralTerrain = Procedural::Terrain(320, 320);
	proceduralTerrain.setTextureData(device, 16, 16, { "../Textures/Lava/diffuse.jpg", "../Textures/Lava/normal.jpg" });

	proceduralTerrain.SetUp(device);

	isTerGenerated = true;
}



void EarthLevel::draw(const RenderContext& rc)
{
	ParticleUpdateData pud = { SVec3(-5, 2, 5), 1.f, rc.dTime };	//wind direction, wind velocity multiplier and delta time
	pSys.updateStdFunc(&pud);

	_sys->_deviceContext->RSSetViewports(1, &rc.d3d->viewport);				//use default viewport for output dimensions
	rc.d3d->SetBackBufferRenderTarget();					//set default screen buffer as output target
	rc.d3d->BeginScene(rc.d3d->clearColour);				//clear colour and depth buffer

	if (isTerGenerated)
	{
		proceduralTerrain.Draw(dc, rc.shMan->shaderTerNorm, *rc.cam, pointLight, rc.elapsed);
	}

	rc.shMan->shaderMaze.SetShaderParameters(dc, maze.model, *rc.cam, pointLight, rc.elapsed, mazeDiffuseMap, mazeNormalMap);
	maze.model.Draw(dc, rc.shMan->shaderMaze);
	rc.shMan->shaderMaze.ReleaseShaderParameters(dc);

	rc.d3d->TurnOffCulling();
	rc.d3d->SwitchDepthToLessEquals();
	rc.shMan->shaderSkybox.SetShaderParameters(dc, skybox.transform, *rc.cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(dc, rc.shMan->shaderSkybox);
	rc.shMan->shaderSkybox.ReleaseShaderParameters(dc);
	rc.d3d->SwitchDepthToDefault();
	rc.d3d->TurnOnCulling();

	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumAir.SetShaderParameters(dc, will, *rc.cam, rc.elapsed);
	will.Draw(dc, rc.shMan->shVolumAir);
	rc.d3d->TurnOffAlphaBlending();

	//move out of here
	if (!_sys->_controller.isFlying())
	{
		SVec3 oldPos = _sys->_renderer._cam.GetCameraMatrix().Translation();
		float newHeight = proceduralTerrain.getHeightAtPosition(_sys->_renderer._cam.GetCameraMatrix().Translation());
		SMatrix newMat = _sys->_renderer._cam.GetCameraMatrix();
		Math::SetTranslation(newMat, SVec3(oldPos.x, newHeight, oldPos.z));
		_sys->_renderer._cam.GetCameraMatrix() = newMat;
	}

	rc.d3d->EndScene();
}



void FireLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(5.f));
	Math::Translate(will.transform, SVec3(2, 35, 60));

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(333.f, 666.f, 999.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)

	Texture terrainTex;
	auto fltVec = terrainTex.generateRidgey(256, 256, 0.f, 1.61803f, 0.5793f, 1.f, 6u);	//auto fltVec = tempTex.generateTurbulent(256, 256, 1.f, 1.61803, 0.5793f, 6u);


	//terrain generation
	terrain.setScales(4, 100, 4);
	terrain.GenFromTexture(terrainTex.w, terrainTex.h, fltVec);
	terrain.Mesa(SVec2(512), 384, 128, -256);
	terrain.CircleOfScorn(SVec2(768, 768), 40.f, PI * 0.01337f, 2 * PI, 64, 1.2 * PI);
	terrain.setOffset(0, 128, 0);

	terrain.setTextureData(device, 10, 10, { "../Textures/LavaCracks/diffuse.png", "../Textures/LavaCracks/normal.png" });
	terrain.SetUp(device);

	island = Procedural::Terrain(128, 128, SVec3(4, 300, 4));
	island.Mesa(SVec2(128), 32, 64, 128);
	island.setOffset(512, 0, 512);
	island.setTextureData(device, 10, 10, { "../Textures/LavaIntense/diffuse.jpg", "../Textures/LavaIntense/normal.jpg" });
	island.SetUp(device);

	lavaSheet = Procedural::Terrain(256, 256, SVec3(4, 2, 4));
	lavaSheet.setTextureData(device, 10, 10, { "../Textures/LavaIntense/diffuse.jpg", "../Textures/LavaIntense/normal.jpg" });
	lavaSheet.setOffset(0, 32, 0);
	lavaSheet.SetUp(device);


	//textures
	hexDiffuseMap.LoadFromFile("../Textures/Crymetal/diffuse.jpg");
	hexDiffuseMap.Setup(device);
	hexNormalMap.LoadFromFile("../Textures/Crymetal/normal.jpg");
	hexNormalMap.Setup(device);


	//hexer
	hexer.init();
	hexer.addPlatform(hexer.root, 0);
	for (int i = 0; i < 30; ++i)
	{
		hexer.addPlatform(hexer.platforms.back(), i % 6);
	}


	Procedural::Geometry hex;
	hex.GenHexaprism(30.f, 10.f);
	hexModel.meshes.push_back(Mesh(hex, device));
}



void FireLevel::procGen()
{
	Procedural::Geometry hex;
	std::vector<Procedural::Geometry> hexes = hex.GenHexGrid(30.f, 10.f, 2);
	for (auto& h : hexes)
		hexCluster.meshes.emplace_back(h, device);

	isTerGenerated = true;
}



void FireLevel::draw(const RenderContext& rc)
{
	dc->RSSetViewports(1, &rc.d3d->viewport);				//use default viewport for output dimensions
	rc.d3d->SetBackBufferRenderTarget();					//set default screen buffer as output target
	rc.d3d->BeginScene(rc.d3d->clearColour);				//clear colour and depth buffer

	//rc.shMan->shaderLight.SetShaderParameters(dc, hexCluster, *rc.cam, pointLight, rc.dTime);
	//hexCluster.Draw(dc, rc.shMan->shaderLight);

	terrain.Draw(dc, rc.shMan->shaderTerNorm, *rc.cam, pointLight, rc.elapsed);
	island.Draw(dc, rc.shMan->shaderTerNorm, *rc.cam, pointLight, rc.elapsed);
	lavaSheet.Draw(dc, rc.shMan->shaderTerNorm, *rc.cam, pointLight, rc.elapsed);

	if (isTerGenerated)
	{
		rc.shMan->shaderMaze.SetShaderParameters(dc, hexCluster, *rc.cam, pointLight, rc.dTime, hexDiffuseMap, hexNormalMap);
		hexCluster.Draw(dc, rc.shMan->shaderMaze);
	}

	rc.d3d->TurnOffCulling();
	rc.d3d->SwitchDepthToLessEquals();
	rc.shMan->shaderSkybox.SetShaderParameters(dc, skybox.transform, *rc.cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(dc, rc.shMan->shaderSkybox);
	rc.shMan->shaderSkybox.ReleaseShaderParameters(dc);
	rc.d3d->SwitchDepthToDefault();
	rc.d3d->TurnOnCulling();

	hexer.update(rc.dTime);
	for (auto& p : hexer.platforms)
	{
		hexModel.transform = SMatrix::CreateTranslation(p.position);
		rc.shMan->shaderMaze.SetShaderParameters(dc, hexModel, *rc.cam, pointLight, rc.dTime, hexDiffuseMap, hexNormalMap);
		hexModel.Draw(dc, rc.shMan->shaderMaze);
	}


	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumFire.SetShaderParameters(dc, will, *rc.cam, rc.elapsed);
	will.Draw(dc, rc.shMan->shVolumFire);
	rc.d3d->TurnOffAlphaBlending();

	rc.d3d->EndScene();
}


void WaterLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

	cubeMapper.Init(device);

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(5.f));
	Math::Translate(will.transform, SVec3(2, 35, 60));

	modBall.LoadModel(device, "../Models/ball.fbx");

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(0, 500.f, 0, 1.0f));

	Procedural::Terrain valley = Procedural::Terrain(256, 256, SVec3(1.f, 50.f, 1.f));

	terrainsMap.insert(std::map<std::string, Procedural::Terrain>::value_type("valley", valley));
}



void WaterLevel::draw(const RenderContext& rc)
{
	rc.d3d->BeginScene(rc.d3d->clearColour);

	dc->RSSetViewports(1, &(cubeMapper.cm_viewport));
	cubeMapper.UpdateCams(modBall.transform.Translation());

	for (int i = 0; i < 6; i++)
	{
		dc->ClearRenderTargetView(cubeMapper.cm_rtv[i], cubeMapper.clearCol);
		dc->ClearDepthStencilView(cubeMapper.cm_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		dc->OMSetRenderTargets(1, &cubeMapper.cm_rtv[i], cubeMapper.cm_depthStencilView);

		rc.d3d->TurnOffCulling();
		rc.d3d->SwitchDepthToLessEquals();

		rc.shMan->shaderSkybox.SetShaderParameters(dc, skybox.transform, cubeMapper.getCameraAtIndex(i), rc.dTime, skyboxCubeMapper.cm_srv);
		skybox.Draw(dc, rc.shMan->shaderSkybox);
		rc.shMan->shaderSkybox.ReleaseShaderParameters(dc);

		rc.d3d->SwitchDepthToDefault();
		rc.d3d->TurnOnCulling();
	}

	dc->RSSetViewports(1, &rc.d3d->viewport);
	rc.d3d->SetBackBufferRenderTarget();

	//RENDERING REFLECTION SPHERE
	rc.shMan->shaderCM.SetShaderParameters(dc, modBall, *rc.cam, pointLight, rc.dTime, cubeMapper.cm_srv);
	modBall.Draw(dc, rc.shMan->shaderCM);
	rc.shMan->shaderCM.ReleaseShaderParameters(dc);


	rc.d3d->TurnOffCulling();
	rc.d3d->SwitchDepthToLessEquals();
	rc.shMan->shaderSkybox.SetShaderParameters(dc, skybox.transform, *rc.cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(dc, rc.shMan->shaderSkybox);
	rc.shMan->shaderSkybox.ReleaseShaderParameters(dc);
	rc.d3d->SwitchDepthToDefault();
	rc.d3d->TurnOnCulling();


	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumAir.SetShaderParameters(dc, will, *rc.cam, rc.elapsed);
	will.Draw(dc, rc.shMan->shVolumAir);
	rc.d3d->TurnOffAlphaBlending();

	rc.d3d->EndScene();
}



void AirLevel::init(Systems& sys)
{

}



void AirLevel::draw(const RenderContext& rc)
{

}

#undef dc





///TREE RENDERING
//rc.shMan->shaderTree.SetShaderParameters(dc, treeModel.transform, *rc.cam, pointLight, rc.elapsed);
//treeModel.Draw(dc, rc.shMan->shaderLight);
//rc.shMan->shaderLight.ReleaseShaderParameters(dc);


///PARTICLE SYSTEM RENDERING
/*
std::vector<InstanceData> instanceData(100);

for (int i = 0; i < instanceData.size(); ++i)
	instanceData[i]._m = pSys._particles[i]->transform.Transpose();

rc.shMan->shaderInstanced.UpdateInstanceData(instanceData);
rc.shMan->shaderInstanced.SetShaderParameters(&rc.shMan->spl);
RES.modBall.Draw(deviceContext, rc.shMan->shaderInstanced);
rc.shMan->shaderInstanced.ReleaseShaderParameters(deviceContext);
*/



#pragma region OldLevel
/*OLD LEVEL

//Math::Scale(modBall.transform, SVec3(36.f));
//Math::Translate(modBall.transform, modBallStand.transform.Translation() + SVec3(0.0f, 42.0f, 0.0f));

modTerrain.LoadModel(_device, "../Models/Terrain/NewTerTex.fbx", 50, 50);
Math::Scale(modTerrain.transform, SVec3(2.f));
_terrainModels.push_back(&modTerrain);

modTreehouse.LoadModel(_device, "../Models/Terrain/Treehouse/thouse(formats).fbx");
Math::SetTranslation(modTreehouse.transform, SVec3(0.0f, -50.f, -100.f));
SMatrix treehouseRotation = SMatrix::CreateFromAxisAngle(SVec3::Up, 30.f);
Math::SetRotation(modTreehouse.transform, treehouseRotation);
_terrainModels.push_back(&modTreehouse);

modBallStand.LoadModel(_device, "../Models/ballstand.fbx");
SMatrix modBallStandRotation = SMatrix::CreateFromAxisAngle(SVec3::Right, PI * 0.5f);
Math::SetRotation(modBallStand.transform, modBallStandRotation);
Math::Scale(modBallStand.transform, SVec3(10.f));
Math::Translate(modBallStand.transform, SVec3(300.0f, -35.0f, -295.0f));

modDepths.LoadModel(_device, "../Models/WaterQuad.fbx");
Math::Scale(modDepths.transform, SVec3(120.0f));
Math::Translate(modDepths.transform, SVec3(0.0f, -50.0f, 0.0f));

white.LoadFromFile("../Textures/noiz.png");
white.Setup(device);
perlinTex.LoadFromFile("../Textures/strife.png");
perlinTex.Setup(device);
worley.LoadFromFile("../Textures/worley.png");
worley.Setup(device);

*/

#pragma endregion OldLevel

#pragma region Audio
/*
musicLSystem.reseed("d");
musicLSystem.addRule('d', "Fa");
musicLSystem.addRule('e', "Gb");
musicLSystem.addRule('f', "Ac");
musicLSystem.addRule('g', "bD");
musicLSystem.addRule('a', "cE");
musicLSystem.addRule('b', "dF");
musicLSystem.addRule('c', "Eg");

musicLSystem.addRule('D', "aD");
musicLSystem.addRule('E', "gB");
musicLSystem.addRule('F', "aC");
musicLSystem.addRule('G', "Bd");
musicLSystem.addRule('A', "Ce");
musicLSystem.addRule('B', "Df");
musicLSystem.addRule('C', "eG");

musicLSystem.rewrite(6);

std::string lSystemNotes = musicLSystem.getString();
std::vector<std::string> notes;
for (char lsn : lSystemNotes)
	notes.push_back(std::string(1, lsn));

audio.init();
audio.storeSequence(notes);

	///RENDERING WATER
	shaderWater.SetShaderParameters(deviceContext, modDepths, rc.cam->GetViewMatrix(), rc.cam->GetProjectionMatrix(),
		dirLight, rc.cam->GetCameraMatrix().Translation(), dTime, white.srv);
	modDepths.Draw(deviceContext, shaderWater);
	shaderWater.ReleaseShaderParameters(deviceContext);


	///RENDERING CLOUD
	shaderStrife.SetShaderParameters(deviceContext, modStrife, rc.cam->GetViewMatrix(), rc.cam->GetProjectionMatrix(),
		dirLight, rc.cam->GetCameraMatrix().Translation(), dTime, white.srv, perlinTex.srv, worley.srv, offScreenTexture._view);
	modStrife.Draw(deviceContext, shaderStrife);
	shaderStrife.ReleaseShaderParameters(deviceContext);

	///RENDERING UI
	//_rekt->draw(deviceContext, shaderHUD, offScreenTexture.srv);
*/
#pragma endregion Audio

#pragma region shadowMatrix

/*
SVec3 lookAtPoint = SVec3(0.f, 100.0f, 0.0f);
SVec3 LVDIR = lookAtPoint - SVec3(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z);
LVDIR.Normalize();
SVec3 LVR = LVDIR.Cross(SVec3::Up);
LVR.Normalize();
SVec3 LVUP = LVR.Cross(LVDIR);
LVUP.Normalize();

dirLight = DirectionalLight(lightData, SVec4(LVDIR.x, LVDIR.y, LVDIR.z, 0.0f));

offScreenTexture.Init(device, ostW, ostH);
offScreenTexture._view = DirectX::XMMatrixLookAtLH(SVec3(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z), lookAtPoint, LVUP);
offScreenTexture._lens = DirectX::XMMatrixOrthographicLH((float)ostW, (float)ostH, 1.0f, 1000.0f);
*/

#pragma endregion shadowMatrix



/*
///Diamond square testing
//proceduralTerrain.GenWithDS(SVec4(0.f, 10.f, 20.f, 30.f), 4u, 0.6f, 10.f);

///Cellular automata testing
//proceduralTerrain.CellularAutomata(0.5f, 0);

///Noise testing	-SVec3(4, 100, 4) scaling with these fbm settings looks great for perlin
//perlin.generate2DTexturePerlin(512, 512, 16.f, 16.f);	//
//perlin.generate2DTextureFBM(256, 256, 1, 1., 4u, 2.1039f, .517f, true);	//(256, 256, 1.f, 1.f, 3, 2.f, .5f);
//proceduralTerrain.GenFromTexture(perlin._w, perlin._h, perlin.getFloatVector());
//perlin.writeToFile("C:\\Users\\metal\\Desktop\\Uni\\test.png");



///Ridge/turbluent noise testing - looks quite nice actually
//Texture tempTex;
//auto fltVec = tempTex.generateTurbulent(256, 256, 1.f, 1.61803, 0.5793f, 6u);
//auto fltVec = tempTex.generateRidgey(256, 256, 0.f, 1.61803f, 0.5793f, 1.f, 6u);
//Texture::WriteToFile("C:\\Users\\metal\\Desktop\\Uni\\test.png", tempTex.w, tempTex.h, 1, tempTex.data, 0);
//proceduralTerrain.GenFromTexture(tempTex.w, tempTex.h, fltVec);


///Terrain deformation testng
//proceduralTerrain.fault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 10.f);
//proceduralTerrain.TerraSlash(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 6.f, 64, 0.9f);
//proceduralTerrain.Fault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 10.f);
//proceduralTerrain.NoisyFault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), -20.f);
//proceduralTerrain.NoisyFault(SRay(SVec3(75.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), +15.f);
//proceduralTerrain.TerraSlash(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 6.f, 64, 0.9f);
//proceduralTerrain.CircleOfScorn(SVec2(proceduralTerrain.getNumCols() / 2, proceduralTerrain.getNumRows() / 2), 40.f, PI * 0.01337f, .5f, 64);
//proceduralTerrain.Smooth(3);

///Voronoi tests - shatter stores an array of the same size as the vertPositions vector, each member containing the index of the closest seed
//Procedural::Voronoi v;
//v.init(25, proceduralTerrain.getNumCols(), proceduralTerrain.getNumRows());
//std::vector<SVec2> vertPositions = proceduralTerrain.getHorizontalPositions();
//v.shatter(vertPositions);

///L-systems testing
linden.reseed("F");
linden.addRule('F', "FF+[+F-F-F]*-[-F+F+F]/"); //"[-F]*F[+F][/F]"	//"F[+F]F[-F]+F" for planar		//"FF+[+F-F-F]*-[-F+F+F]/"
//linden.addRule('F', "F[+F]F[-F]+F");

//linden.reseed("F+F+F+F");
//linden.addRule('F', "FF+F-F+F+FF");

linden.rewrite(4);

float liangle = PI * 0.138888f;		//liangle = PI * .5f;

treeModel = linden.genModel(device, 6.99f, 1.f, .7f, .7f, liangle, liangle);

//Math::RotateMatByMat(treeModel.transform, SMatrix::CreateRotationX(-PI * .5f));
//linden.genVerts(20.f, 0.8f, PI * 0.16666f, PI * 0.16666f);	linden.setUp(_device);
*/

/*
modStrife.LoadModel(device, "../Models/WaterQuad.fbx");
Math::Scale(modStrife.transform, SVec3(15.0f));
Math::RotateMatByMat(modStrife.transform, SMatrix::CreateFromAxisAngle(SVec3::Right, PI));
Math::Translate(modStrife.transform, SVec3(-200.f, 200.0f, -200.0f));

modWaterQuad.LoadModel(device, "../Models/WaterQuad.fbx");
*/