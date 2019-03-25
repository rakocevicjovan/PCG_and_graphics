#include "ResourceManager.h"



ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager()
{
}



void ResourceManager::init(ID3D11Device* device)
{
	_device = device;
	_level.init(device);
}


void Level::init(ID3D11Device* device)
{
	modStrife.LoadModel(device, "../Models/WaterQuad.fbx");
	Math::Scale(modStrife.transform, SVec3(15.0f));
	Math::RotateMatByMat(modStrife.transform, SMatrix::CreateFromAxisAngle(SVec3::Right, PI));
	Math::Translate(modStrife.transform, SVec3(-200.f, 200.0f, -200.0f));

	modSkybox.LoadModel(device, "../Models/Skysphere.fbx");
	Math::Scale(modSkybox.transform, SVec3(10.0f));
	modWaterQuad.LoadModel(device, "../Models/WaterQuad.fbx");

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(5.f));
	Math::Translate(will.transform, SVec3(2, 35, 60));

	modBall.LoadModel(device, "../Models/ball.fbx");
	/*
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
	*/


	///LIGHT DATA, SHADOW MAP AND UI INITIALISATION
	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);

	pointLight = PointLight(lightData, SVec4(0.f, 0.f, 1000.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)


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

	postProcessTexture.Init(device, ostW, ostH);

	mazeDiffuseMap.LoadFromFile("../Textures/Rock/diffuse.jpg");
	mazeDiffuseMap.Setup(device);
	mazeNormalMap.LoadFromFile("../Textures/Rock/normal.jpg");
	mazeNormalMap.Setup(device);

	/*NOISES
	white.LoadFromFile("../Textures/noiz.png");
	white.Setup(device);
	perlinTex.LoadFromFile("../Textures/strife.png");
	perlinTex.Setup(device);
	worley.LoadFromFile("../Textures/worley.png");
	worley.Setup(device);
	*/

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
	*/
#pragma endregion Audio

}