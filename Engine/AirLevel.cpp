#include "AirLevel.h"
#include "Systems.h"
#include "Chaos.h"



void AirLevel::init(Systems& sys)
{
	skybox.LoadModel(S_DEVICE, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(S_DEVICE, "../Textures/day.dds");
	
	auto fltVec = Texture::generateRidgey(256, 256, 1.f, 1.61803f, 0.5793f, 1.f, 6u);
	barrensTerrain.setScales(4, 256, 4);
	barrensTerrain.GenFromTexture(256, 256, fltVec);
	barrensTerrain.Mesa(SVec2(128, 128), 128, 96, 150);
	barrensTerrain.CircleOfScorn(SVec2(768, 768), 40.f, PI * 0.01337f, 5, 64, PI * 1.25);
	barrensTerrain.Smooth(5);
	//barrensTerrain.Tumble(.5f, .015f);
	barrensTerrain.setOffset(-512, 0, -512);
	std::vector<std::string> terTexes = 
	{
	"../Textures/Grass/diffuse.jpg", "../Textures/Snow/diffuse.jpg", "../Textures/Rock/diffuse.jpg",
	"../Textures/Grass/normal.jpg", "../Textures/Snow/normal.jpg", "../Textures/Rock/normal.jpg"
	};
	barrensTerrain.setTextureData(S_DEVICE, 20, 20, terTexes);
	barrensTerrain.SetUp(S_DEVICE);
	barrens = Model(barrensTerrain, S_DEVICE);

	tornado.LoadModel(S_DEVICE, "../Models/boxnado.obj");
	tornado.transform = SMatrix::CreateScale(128, 512, 128);
	tornado.transform *= SMatrix::CreateTranslation(0, 0, 0);
	
	goal = SVec3(-384, 0, -384);
	goal.y = barrensTerrain.getHeightAtPosition(goal) + 33.f;
	will.LoadModel(S_DEVICE, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(10.f));
	will.transform *= SMatrix::CreateTranslation(goal);

	LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
	pointLight = PointLight(lightData, SVec4(0, 500, 0, 1));
	dirLight = DirectionalLight(lightData, SVec4(0, -1, 0, 1));

	//light "camera matrix"
	lightView = SMatrix::CreateFromAxisAngle(SVec3(1, 0, 0), PI * 0.5) * SMatrix::CreateTranslation(SVec3(pointLight.pos));
	lightView.Invert();		//get the view matrix of the light
	lightView.Transpose();	//transpose so it doesn't have to be transposed by the shader class each frame

	//@WARNING when I update the rendering completely, this won't go...
	glider.LoadModel(S_DEVICE, "../Models/glider/rrrr.fbx");
	player.a.gc.model = &glider;
	player.a.transform = SMatrix::CreateScale(0.05) * SMatrix::Identity;
	player.a.transform *= SMatrix::CreateRotationY(PI * 1.25);
	player.a.transform *= SMatrix::CreateTranslation(SVec3(256, 400, 256));
	player.a.gc.shader = &shady.light;
	player.con = _sys._controller;
	player.cam = randy._cam;
	player.cam.SetTranslation(player.a.transform.Translation() + player.a.transform.Forward() * SVec3(0, 1, 2));
	

	segmentModel.LoadModel(S_DEVICE, "../Models/Ball.fbx");
	segmentModel.transform = SMatrix::CreateScale(15);

	dragon.init(17, SVec3(0, 0, 200));
	instanceData.resize(17);

	winTex = Texture(S_DEVICE, "../Textures/Victory.png");
	winScreenNode = winScreen.AddUINODE(S_DEVICE, winScreen.getRoot(), SVec2(0, 0), SVec2(1, 1));
}



void AirLevel::update(const RenderContext& rc)
{
	ProcessSpecialInput(rc.dTime);
	player.UpdateCamTP(rc.dTime);
	dragon.update(rc.dTime, windDir * windInt, player.getPosition());

	for (int i = 0; i < dragon.springs.size(); ++i)
		instanceData[i]._m = dragon.springs[i].transform.Transpose();

	shady.dragon.UpdateInstanceData(instanceData);

	SVec3 pp(player.a.transform.Translation());
	SVec2 tp(tornado.transform.Translation().x, tornado.transform.Translation().z);
	float ph = pp.y;

	if(SVec2::Distance(SVec2(pp.x, pp.z), tp) < 64. &&  ph < 768.f)
		Math::Translate(player.a.transform, SVec3(0, min(45 * rc.dTime, 768.f - ph * 1.02), 0));
	else
		Math::Translate(player.a.transform, SVec3(0, -8.f * rc.dTime, 0));

	bool dead = ph <= barrensTerrain.getHeightAtPosition(pp) 
		|| 
		(SVec3::Distance(pp, dragon.springs[0].transform.Translation()) < 32.f);

	if(dead)
	{
		player.a.transform = SMatrix::CreateScale(0.05) * SMatrix::Identity;
		player.a.transform *= SMatrix::CreateRotationY(PI * 1.25);
		player.a.transform *= SMatrix::CreateTranslation(SVec3(256, 400, 256));
	}

	if (SVec3::Distance(pp, goal) < 30.f)
	{
		++winCount;

		Chaos c;
		c.setRange(-1, 1);
		float x = c.rollTheDice() * 512.;
		float z = c.rollTheDice() * 512.;

		goal = SVec3(x, 0, z);
		goal.y = barrensTerrain.getHeightAtPosition(goal) + 33.f;
		Math::SetTranslation(will.transform, goal);
	}
}



void AirLevel::draw(const RenderContext& rc)
{
	if (winCount >= 3)
	{
		rc.d3d->ClearColourDepthBuffers();
		winScreen.draw(context, shady.HUD, winTex.srv);
		rc.d3d->EndScene();
		return;
	}

	rc.d3d->ClearColourDepthBuffers();
	
	_sys._D3D.setRSSolidNoCull();
	shady.light.SetShaderParameters(context, player.a.transform, player.cam, pointLight, rc.elapsed);
	glider.Draw(context, shady.light);
	shady.light.ReleaseShaderParameters(context);
	_sys._D3D.setRSSolidCull();

	shady.terrainMultiTex.SetShaderParameters(context, barrens.transform, player.cam, pointLight, rc.dTime);
	barrens.Draw(context, shady.terrainMultiTex);

	randy.RenderSkybox(player.cam, skybox, skyboxCubeMapper);

	_sys._D3D.TurnOnAlphaBlending();

	shady.dragon.SetShaderParameters(context, segmentModel, player.cam, pointLight, rc.dTime);
	segmentModel.Draw(context, shady.dragon);
	shady.dragon.ReleaseShaderParameters(context);

	shady.shVolumTornado.SetShaderParameters(context, tornado, player.cam, rc.elapsed);
	tornado.Draw(context, shady.shVolumTornado);

	rc.shMan->shVolumAir.SetShaderParameters(context, will, player.cam, rc.elapsed);
	will.Draw(context, rc.shMan->shVolumAir);

	_sys._D3D.TurnOffAlphaBlending();

	rc.d3d->EndScene();
}

/*
gales.r[0] = SVec4(0, 100, 100, 13);
gales.r[1] = SVec4(100, 100, 33, 15);
gales.r[2] = SVec4(0, 100, 120, 12);
gales.r[3] = SVec4(-170, 100, 15, 17);

shady.shVolumScreen.SetShaderParameters(context, player.cam, gales, rc.elapsed);
shady.shVolumScreen.screenQuad->draw(context, shady.shVolumScreen);
shady.shVolumScreen.ReleaseShaderParameters(context);
*/