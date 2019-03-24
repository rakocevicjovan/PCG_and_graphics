#include "Renderer.h"
#include "InputManager.h"

Renderer::Renderer() : proceduralTerrain()
{
	_D3D = 0;
	drawUI = false;
}


Renderer::~Renderer() {}

#define RES _resMan._level

bool Renderer::Initialize(int windowWidth, int windowHeight, HWND hwnd, InputManager& inMan)
{
	_inMan = &inMan;
	bool result;

	// Create the Direct3D object.
	_D3D = new D3DClass;
	if(!_D3D)
		return false;

	// Initialize the Direct3D object.
	if(!_D3D->Initialize(windowWidth, windowHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR)){
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	_device = _D3D->GetDevice();
	_deviceContext = _D3D->GetDeviceContext();

	_colEngine.init(_device, _deviceContext);

	shMan.init(_device, hwnd);
	_resMan.init(_device);

	_rekt = new Rekt(_device, _deviceContext);
	screenRect = _rekt->AddUINODE(_rekt->getRoot(), SVec2(0.75f, 0.75f), SVec2(0.25f, 0.25f));

	maze.Init(10, 10, 32.f);
	maze.CreateModel(_device);

	_colEngine.registerModel(&(maze.model), BVT_AABB);

	///CAMERA INITIALISATION
	SMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_D3D->_fieldOfView, _D3D->_screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	_cam = Camera(SMatrix::Identity, projectionMatrix);
	_controller = Controller(&inMan);
	_cam._controller = &_controller;

	_colEngine.registerController(_controller);	//works both ways
	
	///CUBE MAPS SETUP
	cubeMapper.Init(_device);
	shadowCubeMapper.Init(_device);
	skyboxCubeMapper.LoadFromFiles(_device, "../Textures/night.dds");


	pSys.init(_device, 100, SVec3(0, 0, 100), "../Models/ball.fbx");
	
	lambda = [this](PUD* pud) -> void  
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
	
	lambda1 = [this](PUD* pud) -> void
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

	pSys.setUpdateFunction(lambda);
	pSys.setShader(&shMan.shaderBase);

	shMan.shaderVolumetric.setLightData(_deviceContext, RES.pointLight);

	//_colEngine.registerModel(&RES.will, BoundingVolumeType::BVT_AABB);

	return true;
}


bool Renderer::Frame(float dTime){
	

	ProcessSpecialInput();
	elapsed += dTime;

	if (!_controller.isFlying())
	{
		SVec3 oldPos = _cam.GetCameraMatrix().Translation();
		float newHeight = proceduralTerrain.getHeightAtPosition(_cam.GetCameraMatrix().Translation());
		SMatrix newMat = _cam.GetCameraMatrix();
		Math::SetTranslation(newMat, SVec3(oldPos.x, newHeight, oldPos.z));
		_cam.SetCameraMatrix(newMat);
	}
	_cam.update(dTime);

	Math::SetTranslation(RES.modSkybox.transform, _cam.GetCameraMatrix().Translation());

	return RenderFrame(dTime);
}



bool Renderer::RenderFrame(float dTime)
{
	PUD pud = { SVec3(-5, 2, 5), 1.f, dTime };
	pSys.updateStdFunc(&pud);

	shMan.spl.deltaTime = dTime;
	shMan.spl.deviceContext = _deviceContext;
	shMan.spl.dLight = &(_resMan._level.pointLight);
	shMan.spl.eyePos = &(_cam.GetCameraMatrix().Translation());
	shMan.spl.model = &pSys._model;
	shMan.spl.proj = &(_cam.GetProjectionMatrix());
	shMan.spl.view = &(_cam.GetViewMatrix());

	_deviceContext->RSSetViewports(1, &_D3D->viewport);
	_D3D->SetBackBufferRenderTarget();
	_D3D->BeginScene(clearColour);


	///RENDERING OLD TERRAIN 
	/*
	for (auto tm : _terrainModels) {
		shaderShadow.SetShaderParameters(_deviceContext, *tm, _cam.GetViewMatrix(), offScreenTexture._view, _cam.GetProjectionMatrix(),
			offScreenTexture._lens, pointLight, _cam.GetCameraMatrix().Translation(), offScreenTexture.srv);
		tm->Draw(_deviceContext, shaderShadow);
		shaderShadow.ReleaseShaderParameters(_deviceContext);
	}
	*/

	SMatrix identityMatrix = SMatrix::Identity;

	_D3D->TurnOffCulling();
	_D3D->SwitchDepthToLessEquals();
	shMan.shaderSkybox.SetShaderParameters(_deviceContext, RES.modSkybox, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
		_cam.GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
	RES.modSkybox.Draw(_deviceContext, shMan.shaderSkybox);
	shMan.shaderSkybox.ReleaseShaderParameters(_deviceContext);
	_D3D->SwitchDepthToDefault();
	_D3D->TurnOnCulling();

	if (isTerGenerated) 
	{
		_D3D->TurnOnAlphaBlending();

		/*
		proceduralTerrain.Draw(_deviceContext, shMan.shaderPerlin,
			identityMatrix, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
			_resMan._level.pointLight, elapsed, _cam.GetCameraMatrix().Translation());
		*/

		proceduralTerrain.Draw(_deviceContext, shMan.shaderTerrain,
			identityMatrix, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
			_resMan._level.pointLight, elapsed, _cam.GetCameraMatrix().Translation());
		
		_D3D->TurnOffAlphaBlending();

		/*
		linden.draw(_deviceContext, shMan.shaderLight,
			identityMatrix, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
			pointLight, dTime, _cam.GetCameraMatrix().Translation());
		*/


		shMan.shaderLight.SetShaderParameters(_deviceContext, 
			treeModel, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(), 
			RES.pointLight, _cam.GetCameraMatrix().Translation(), dTime);
		treeModel.Draw(_deviceContext, shMan.shaderLight);
		shMan.shaderLight.ReleaseShaderParameters(_deviceContext);

	}

	shMan.shaderMaze.SetShaderParameters(_deviceContext, maze.model, _cam, elapsed);
	maze.model.Draw(_deviceContext, shMan.shaderMaze);

	//shMan.shaderMaze.SetShaderParameters(_deviceContext, _colEngine._colModels[0], _cam, elapsed);
	//_colEngine._colModels[0].Draw(_deviceContext, shMan.shaderMaze);

	/*
	shMan.shaderLight.SetShaderParameters(_deviceContext,
		maze.model, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
		RES.pointLight, _cam.GetCameraMatrix().Translation(), dTime);
	maze.model.Draw(_deviceContext, shMan.shaderLight);
	shMan.shaderLight.ReleaseShaderParameters(_deviceContext);
	*/

	/*
 	std::vector<InstanceData> instanceData(100);

	for (int i = 0; i < instanceData.size(); ++i)
		instanceData[i]._m = pSys._particles[i]->transform.Transpose();

	shMan.shaderInstanced.UpdateInstanceData(instanceData);
	shMan.shaderInstanced.SetShaderParameters(&shMan.spl);
	RES.modBall.Draw(_deviceContext, shMan.shaderInstanced);
	shMan.shaderInstanced.ReleaseShaderParameters(_deviceContext);
	*/

	_D3D->TurnOnAlphaBlending();
	shMan.shaderVolumetric.SetShaderParameters(_deviceContext, RES.will, _cam, elapsed);
	_resMan._level.will.Draw(_deviceContext, shMan.shaderVolumetric);
	_D3D->TurnOffAlphaBlending();
	

	///rendering water and clouds
	/*
	///RENDERING WATER
	shaderWater.SetShaderParameters(_deviceContext, modDepths, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
		dirLight, _cam.GetCameraMatrix().Translation(), dTime, white.srv);
	modDepths.Draw(_deviceContext, shaderWater);
	shaderWater.ReleaseShaderParameters(_deviceContext);

	*/
	///RENDERING CLOUD
	/*
	shaderStrife.SetShaderParameters(_deviceContext, modStrife, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
		dirLight, _cam.GetCameraMatrix().Translation(), dTime, white.srv, perlinTex.srv, worley.srv, offScreenTexture._view);
	modStrife.Draw(_deviceContext, shaderStrife);
	shaderStrife.ReleaseShaderParameters(_deviceContext);
	*/

	//_rekt->draw(_deviceContext, shaderHUD, offScreenTexture.srv);

	_D3D->EndScene();
	return true;
}



void Renderer::Shutdown() 
{
	if (_D3D) 
	{
		_D3D->Shutdown();
		delete _D3D;
		_D3D = 0;
	}
}



void Renderer::OutputFPS(float dTime) 
{
	std::ostringstream ss;
	ss << "Frame time: " << 1.0f / dTime << "\n";
	std::string s(ss.str());
	OutputDebugStringA(ss.str().c_str());
}



void Renderer::ProcessSpecialInput() 
{
	if (_inMan->IsKeyDown(VK_SPACE)) 
	{

		///TERRAIN GENERATION
		//proceduralTerrain = Procedural::Terrain(2, 2, SVec3(10, 30, 10));
		proceduralTerrain.setScales(1, 50, 1);

		std::vector<std::string> texNames = 
		{
			"../Textures/Biomes/grass.png", "../Textures/Biomes/ice_grass.jpg", 
			"../Textures/Biomes/snow.jpg", "../Textures/Biomes/cliff.jpg"
		};

		proceduralTerrain.setTextureData(_device, 2.56f, 2.56f, texNames);

		///Diamond square testing
		//proceduralTerrain.GenWithDS(SVec4(0.f, 10.f, 20.f, 30.f), 4u, 0.6f, 10.f);

		///Cellular automata testing
		//proceduralTerrain.CellularAutomata(0.5f, 0);

		///Noise testing	-SVec3(4, 100, 4) scaling with these fbm settings looks great for perlin
		//perlin.generate2DTexturePerlin(512, 512, 16.f, 16.f);	//
		//perlin.generate2DTextureFBM(256, 256, 1, 1., 4u, 2.1039f, .517f, true);	//(256, 256, 1.f, 1.f, 3, 2.f, .5f);
		//proceduralTerrain.GenFromTexture(perlin._w, perlin._h, perlin.getFloatVector());
		//perlin.writeToFile("C:\\Users\\metal\\Desktop\\Uni\\test.png");



		///Ridge/turbluent noise testing
		Texture tempTex;
		//auto fltVec = tempTex.generateTurbulent(256, 256, 1.f, 1.61803, 0.5793f, 6u);
		auto fltVec = tempTex.generateRidgey(256, 256, 0.f, 1.61803f, 0.5793f, 1.f, 6u);
		//Texture::WriteToFile("C:\\Users\\metal\\Desktop\\Uni\\test.png", tempTex.w, tempTex.h, 1, tempTex.data, 0);
		proceduralTerrain.GenFromTexture(tempTex.w, tempTex.h, fltVec);


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

		proceduralTerrain.SetUp(_device);
		std::vector<Procedural::Terrain*> terrains;
		terrains.push_back(&proceduralTerrain);

		isTerGenerated = true;

		///L-systems testing
		linden.reseed("F");
		linden.addRule('F', "FF+[+F-F+F]*-[-F+F-F]/"); //"[-f]*f[+f][/f]");		//f[+f]f[-f]+f for planar looks ok
		
		//linden.reseed("F+F+F+F");
		//linden.addRule('F', "FF+F-F+F+FF");

		linden.rewrite(4);

		float liangle = PI * 0.138888f;
		//liangle = PI * .5f;
		treeModel = linden.genModel(_device, 10.f, 2.f, .9f, .8f, liangle, liangle);

		//linden.genVerts(20.f, 0.8f, PI * 0.16666f, PI * 0.16666f);	linden.setUp(_device);	
	}

	if(_inMan->IsKeyDown((short)'F'))
	{
		_controller.toggleFly();
	}
}








#pragma region oldScene

/*
///PROJECT TEXTURE
SMatrix texView = DirectX::XMMatrixLookAtLH(SVec3(0.0f, 0.0f, -1.0f), SVec3(0.0f, 0.0f, 0.0f), SVec3::Up);
shaderPT.SetShaderParameters(_deviceContext, modTerrain, cam.GetViewMatrix(), cam.GetViewMatrix(), cam.GetProjectionMatrix(),
							cam.GetProjectionMatrix(), _lights[0], cam.GetCameraMatrix().Translation(), dTime, offScreenTexture.srv);
modTerrain.Draw(_deviceContext, shaderPT);
shaderPT.ReleaseShaderParameters(_deviceContext);
*/

/*

///REFLECTION CUBE MAP START
_deviceContext->RSSetViewports(1, &(cubeMapper.cm_viewport));
cubeMapper.UpdateCams(modBall.transform.Translation());

SMatrix wot = DirectX::XMMatrixInverse(nullptr, cubeMapper.cameras[0]);
Math::SetTranslation(modSkybox.transform, wot.Translation());
for (int i = 0; i < 6; i++) {

	_deviceContext->ClearRenderTargetView(cubeMapper.cm_rtv[i], cubeMapper.clearCol);
	_deviceContext->ClearDepthStencilView(cubeMapper.cm_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	_deviceContext->OMSetRenderTargets(1, &cubeMapper.cm_rtv[i], cubeMapper.cm_depthStencilView);

	shaderLight.SetShaderParameters(_deviceContext, modTerrain, cubeMapper.cameras[i], cubeMapper.lens, pointLight, _cam.GetCameraMatrix().Translation(), dTime);
	modTerrain.Draw(_deviceContext, shaderLight);
	shaderLight.ReleaseShaderParameters(_deviceContext);

	shaderLight.SetShaderParameters(_deviceContext, modTreehouse, cubeMapper.cameras[i], cubeMapper.lens, pointLight, _cam.GetCameraMatrix().Translation(), dTime);
	modTreehouse.Draw(_deviceContext, shaderLight);
	shaderLight.ReleaseShaderParameters(_deviceContext);

	_D3D->TurnOffCulling();
	_D3D->SwitchDepthToLessEquals();


	shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, cubeMapper.cameras[i], cubeMapper.lens,
		_cam.GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
	modSkybox.Draw(_deviceContext, shaderSkybox);
	shaderSkybox.ReleaseShaderParameters(_deviceContext);

	_D3D->SwitchDepthToDefault();
	_D3D->TurnOnCulling();
}
Math::SetTranslation(modSkybox.transform, _cam.GetCameraMatrix().Translation());
///REFLECTION CUBE MAP DONE


///RENDERING UI
_deviceContext->RSSetViewports(1, &_D3D->viewport);
_D3D->SetBackBufferRenderTarget();
_D3D->BeginScene(clearColour);

if (drawUI)
_rekt->draw(_deviceContext, shaderHUD, offScreenTexture.srv);
///RENDERING UI DONE


///RENDERING DEPTH TEXTURE
_deviceContext->RSSetViewports(1, &altViewport);	//to the shadow texture viewport
_deviceContext->OMSetRenderTargets(1, &(offScreenTexture.rtv), _D3D->GetDepthStencilView());	//switch to drawing on ost for the prepass	
_deviceContext->ClearRenderTargetView(offScreenTexture.rtv, ccb);	//then clear it, both the colours and the depth-stencil buffer
_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

for (auto tm : _terrainModels) {
	shaderDepth.SetShaderParameters(_deviceContext, *tm, offScreenTexture._view, offScreenTexture._lens);
	tm->Draw(_deviceContext, shaderDepth);
}

shaderLight.SetShaderParameters(_deviceContext, modBall, offScreenTexture._view, offScreenTexture._lens, pointLight,
	_cam.GetCameraMatrix().Translation(), dTime);
modBall.Draw(_deviceContext, shaderLight);
shaderLight.ReleaseShaderParameters(_deviceContext);

///RENDERING TERRAIN
_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
_deviceContext->RSSetViewports(1, &_D3D->viewport);
_D3D->SetBackBufferRenderTarget();

for (auto tm : _terrainModels) {
	shaderShadow.SetShaderParameters(_deviceContext, *tm, _cam.GetViewMatrix(), offScreenTexture._view, _cam.GetProjectionMatrix(),
		offScreenTexture._lens, pointLight, _cam.GetCameraMatrix().Translation(), offScreenTexture.srv);
	tm->Draw(_deviceContext, shaderShadow);
	shaderShadow.ReleaseShaderParameters(_deviceContext);
}
///RENDERING TERRAIN DONE

///RENDERING WIREFRAME
_D3D->TurnOnAlphaBlending();
shaderWireframe.SetShaderParameters(_deviceContext, modBallStand, _cam.GetViewMatrix(), _cam.GetProjectionMatrix());
modBallStand.Draw(_deviceContext, shaderWireframe);
shaderWireframe.ReleaseShaderParameters(_deviceContext);
_D3D->TurnOffAlphaBlending();
///RENDERING WIREFRAME DONE




///RENDERING REFLECTION SPHERE/*
shaderCM.SetShaderParameters(_deviceContext, modBall, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(), dirLight,
	_cam.GetCameraMatrix().Translation(), dTime, cubeMapper.cm_srv);
modBall.Draw(_deviceContext, shaderCM);
shaderCM.ReleaseShaderParameters(_deviceContext);
///RENDERING REFLECTION SPHERE DONE



///RENDERING SKYBOX
_D3D->TurnOffCulling();
_D3D->SwitchDepthToLessEquals();
shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
	_cam.GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
modSkybox.Draw(_deviceContext, shaderSkybox);
shaderSkybox.ReleaseShaderParameters(_deviceContext);
_D3D->SwitchDepthToDefault();
_D3D->TurnOnCulling();
///RENDERING SKYBOX DONE


_D3D->TurnOnAlphaBlending();





_D3D->TurnOffAlphaBlending();

*/

#pragma endregion oldScene