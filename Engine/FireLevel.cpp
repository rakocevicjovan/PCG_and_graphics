#include "FireLevel.h"



void FireLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(10.f));
	Math::Translate(will.transform, SVec3(768, 138, 768));

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

	//hexer initialization
	float hexRadius = 30.f;
	hexer.init(hexRadius, SVec3(0, 0, 0));

	Procedural::Geometry hex;
	hex.GenHexaprism(hexRadius, 10.f);
	hexModel.meshes.push_back(Mesh(hex, device));


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
	dc->RSSetViewports(1, &rc.d3d->viewport);				//use default viewport for output dimensions
	rc.d3d->SetBackBufferRenderTarget();					//set default screen buffer as output target
	rc.d3d->BeginScene(rc.d3d->clearColour);				//clear colour and depth buffer

	terrain.Draw(dc, rc.shMan->shaderTerNorm, *rc.cam, pointLight, rc.elapsed);

	for (auto& island : _islands) 
	{
		island.Draw(dc, rc.shMan->shaderTerNorm, *rc.cam, pointLight, rc.elapsed);
	}

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

	for (auto& p : hexer._platforms)
	{
		hexModel.transform = SMatrix::CreateTranslation(p.position);
		rc.shMan->shaderMaze.SetShaderParameters(dc, hexModel, *rc.cam, pointLight, rc.dTime, hexDiffuseMap, hexNormalMap);
		hexModel.Draw(dc, rc.shMan->shaderMaze);
	}

	//transparent items
	rc.d3d->TurnOnAlphaBlending();

	rc.shMan->shVolumLava.SetShaderParameters(dc, lavaSheetModel, *rc.cam, rc.elapsed);
	lavaSheetModel.Draw(dc, rc.shMan->shVolumLava);

	rc.shMan->shVolumFire.SetShaderParameters(dc, will, *rc.cam, rc.elapsed);
	will.Draw(dc, rc.shMan->shVolumFire);

	rc.d3d->TurnOffAlphaBlending();

	//finish up
	rc.d3d->EndScene();

	ProcessSpecialInput(rc.dTime);
}