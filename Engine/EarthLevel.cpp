#include "EarthLevel.h"


void EarthLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(5.f));
	Math::Translate(will.transform, SVec3(2, 35, 60));

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(333.f, 666.f, 999.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)

	proceduralTerrain = Procedural::Terrain(320, 320);
	proceduralTerrain.setTextureData(device, 16, 16, { "../Textures/Lava/diffuse.jpg", "../Textures/Lava/normal.jpg" });
	proceduralTerrain.SetUp(device);

	mazeDiffuseMap = Texture(device, "../Textures/Rock/diffuse.jpg");
	mazeNormalMap = Texture(device, "../Textures/Rock/normal.jpg");

	maze.Init(10, 10, 32.f);
	maze.CreateModel(device);

	sys._colEngine.registerModel(maze.model, BoundingVolumeType::BVT_AABB);

	SMatrix goalMat = SMatrix::CreateTranslation(maze.GetRandCellPos());
	pSys.init(&will, 10, goalMat);
	goal = goalMat.Translation();

	particleUpdFunc = [this](ParticleUpdateData* _pud) -> void
	{
		for (int i = 0; i < pSys._particles.size(); ++i)
		{
			SVec3 translation(1, 1, 1);
			translation.x *= sin(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			translation.y *= cos(pSys._particles[i]->age  * ((float)pSys._particles.size() - (float)i));
			translation.z *= cos(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			Math::SetTranslation(pSys._particles[i]->transform, translation * (float)i * -0.33f);
		}
	};

}



void EarthLevel::update(const RenderContext & rc)
{
	updateCam(rc.dTime);
	win(rc.cam->GetPosition());
}



void EarthLevel::draw(const RenderContext& rc)
{
	
	
	rc.d3d->SetBackBufferRenderTarget();

	proceduralTerrain.Draw(context, rc.shMan->terrainNormals, *rc.cam, pointLight, rc.elapsed);

	rc.shMan->dynamicHeightMaze.SetShaderParameters(context, maze.model, *rc.cam, pointLight, rc.elapsed, mazeDiffuseMap, mazeNormalMap);
	maze.model.Draw(context, rc.shMan->dynamicHeightMaze);
	rc.shMan->dynamicHeightMaze.ReleaseShaderParameters(context);

	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumAir.SetShaderParameters(context, will, *rc.cam, rc.elapsed);
	will.Draw(context, rc.shMan->shVolumAir);
	rc.d3d->TurnOffAlphaBlending();

	rc.d3d->EndScene();

	ProcessSpecialInput(rc.dTime);
}