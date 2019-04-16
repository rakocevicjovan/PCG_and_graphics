#include "AirLevel.h"



void AirLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	/*
	Procedural::Terrain barrensTerrain;
	auto fltVec = Texture::generateRidgey(128, 128, 0.f, 1.61803f, 0.5793f, 1.f, 6u);
	barrensTerrain.setScales(4, 128, 4);
	barrensTerrain.GenFromTexture(128, 128, fltVec);
	barrensTerrain.Mesa(SVec2(400), 48, 48, 150);

	barrensTerrain.setOffset(-256, 0, -256);

	std::vector<std::string> terTexes = 
	{	"../Textures/Grass/diffuse.jpg",
		"../Textures/Snow/diffuse.jpg",
		"../Textures/Rock/diffuse.jpg",
		"../Textures/Grass/normal.jpg",
		"../Textures/Snow/normal.jpg",
		"../Textures/Rock/normal.jpg"		
	};

	barrensTerrain.setTextureData(device, 20, 20, terTexes);
	barrensTerrain.SetUp(device);
	
	barrens = Model(barrensTerrain, device);
	*/

	LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
	pointLight = PointLight(lightData, SVec4(0, 500, 0, 1));
	dirLight = DirectionalLight(lightData, SVec4(0, -1, 0, 1));

	//light "camera matrix"
	lightView = SMatrix::CreateFromAxisAngle(SVec3(1, 0, 0), PI * 0.5) * SMatrix::CreateTranslation(SVec3(pointLight.pos));
	lightView.Invert();		//get the view matrix of the light
	lightView.Transpose();	//transpose so it doesn't have to be transposed by the shader class each frame

	worley = Texture(device, "../Textures/worley.png");

	headModel.LoadModel(device, "../Models/Ball.fbx");	//../Models/Dragon/dragonhead.obj
	segmentModel.LoadModel(device, "../Models/Ball.fbx");
	segmentModel.transform = SMatrix::CreateScale(15);

	segmentModel.meshes[0].textures[0] = Texture(device, "../Textures/Quilty/diffuse.jpg");
	//segmentModel.meshes[0].textures.push_back(Texture(device, "../Textures/Quilty/normal.jpg"));

	dragon.init(17, SVec3(0, 0, 200));
	instanceData.resize(17);

	windPipeTexture.Init(device, 1600, 900);
	screenRectangleNode = windPiper.AddUINODE(device, windPiper.getRoot(), SVec2(0, 0), SVec2(1, 1));
	Mesh windPipeQuad = windPiper.getRoot()->children[0]->m;
}



void AirLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers(rc.d3d->clearColour);
	ProcessSpecialInput(rc.dTime);
	updateCam(rc.dTime);

	dragon.update(rc, windDir * windInt, rc.cam->GetPosition());

	for (int i = 0; i < dragon.springs.size(); ++i)
		instanceData[i]._m = dragon.springs[i].transform.Transpose();

	/*
	_sys._D3D.TurnOnAlphaBlending();
	
	shady.strife.SetShaderParameters(context, headModel, *rc.cam, dirLight, rc.elapsed, worley.srv, lightView);
	headModel.Draw(context, shady.strife);
	shady.strife.ReleaseShaderParameters(context);

	shady.terrainMultiTex.SetShaderParameters(context, barrens.transform, *rc.cam, pointLight, rc.dTime);
	barrens.Draw(context, shady.terrainMultiTex);

	_sys._D3D.TurnOffAlphaBlending();
	*/
	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);


	_sys._D3D.TurnOnAlphaBlending();

	//shady.dragon.UpdateInstanceData(instanceData);
	//shady.dragon.SetShaderParameters(context, segmentModel, *rc.cam, pointLight, rc.dTime);
	//segmentModel.Draw(context, shady.dragon);
	//shady.dragon.ReleaseShaderParameters(context);

	shady.shVolumScreen.SetShaderParameters(context, *rc.cam, rc.elapsed);
	shady.shVolumScreen.screenQuad->draw(context, shady.shVolumScreen);
	shady.shVolumScreen.ReleaseShaderParameters(context);

	_sys._D3D.TurnOffAlphaBlending();

	rc.d3d->EndScene();
}
