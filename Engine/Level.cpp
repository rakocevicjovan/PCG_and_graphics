#include "Level.h"
#include "Controller.h"
#include "InputManager.h"
#include <d3d11.h>



Level::Level(Systems& sys) : _sys(sys) {}



void Level::ProcessSpecialInput(float dTime)
{
	sinceLastInput += dTime;

	if (sinceLastInput < .33f)
		return;

	if (_sys._inputManager.isKeyDown(VK_SPACE))
	{
		procGen();
		sinceLastInput = 0;
	}

}


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
	water.SetShaderParameters(deviceContext, modDepths, rc.cam->GetViewMatrix(), rc.cam->GetProjectionMatrix(),
		dirLight, rc.cam->GetCameraMatrix().Translation(), dTime, white.baseSrv);
	modDepths.Draw(deviceContext, water);
	water.ReleaseShaderParameters(deviceContext);


	///RENDERING CLOUD
	strife.SetShaderParameters(deviceContext, modStrife, rc.cam->GetViewMatrix(), rc.cam->GetProjectionMatrix(),
		dirLight, rc.cam->GetCameraMatrix().Translation(), dTime, white.baseSrv, perlinTex.baseSrv, worley.baseSrv, offScreenTexture._view);
	modStrife.Draw(deviceContext, strife);
	strife.ReleaseShaderParameters(deviceContext);

	///RENDERING UI
	//postProcessor->draw(deviceContext, HUD, offScreenTexture.baseSrv);
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

flowerModel = linden.genModel(device, 6.99f, 1.f, .7f, .7f, liangle, liangle);

//Math::RotateMatByMat(flowerModel.transform, SMatrix::CreateRotationX(-PI * .5f));
//linden.genVerts(20.f, 0.8f, PI * 0.16666f, PI * 0.16666f);	linden.setUp(_device);
*/

/*
modStrife.LoadModel(device, "../Models/WaterQuad.fbx");
Math::Scale(modStrife.transform, SVec3(15.0f));
Math::RotateMatByMat(modStrife.transform, SMatrix::CreateFromAxisAngle(SVec3::Right, PI));
Math::Translate(modStrife.transform, SVec3(-200.f, 200.0f, -200.0f));

modWaterQuad.LoadModel(device, "../Models/WaterQuad.fbx");
*/