#include "pch.h"

#include "Engine.h"
#include "GUI.h"

#include "ShaderGenerator.h"
#include "LevelAsset.h"


Engine::Engine() :
	_scrWidth(GetSystemMetrics(SM_CXSCREEN)),
	_scrHeight(GetSystemMetrics(SM_CYSCREEN)),
	_threadPool(std::thread::hardware_concurrency() - 1)
{}


bool Engine::initialize()
{
	_engineWindow.createWindow("Aeolian engine", this, _windowWidth, _windowHeight,
		Window<Engine>::CreationFlags::SHOW_WINDOW |
		Window<Engine>::CreationFlags::START_FOCUSED |
		Window<Engine>::CreationFlags::START_FOREGROUND);

	if (!_D3D.initialize(_windowWidth, _windowHeight, false, _engineWindow.handle(), _engineWindow.fullscreen()))
	{
		OutputDebugStringA("Can't initialize D3D!");
		return false;
	}

	_inputManager.initialize(_engineWindow.handle());
	_defController = Controller(&_inputManager);
	_inputManager.registerController(&_defController);

	if (!_renderer.initialize(_windowWidth, _windowHeight, _D3D))
	{
		OutputDebugStringA("Could not initialize the renderer!");
		return false;
	}

	_shaderCompiler.init(_D3D.getDevice());
	//_matCache.init(&_shaderCache, &_resMan);

	_renderer._cam._controller = &_defController;

	GUI::initDxWin32(_engineWindow.handle(), _D3D.getDevice(), _D3D.getContext());

	// Loads the project configuration data into the project loader, as well as a list of levels associated to the project
	_project.loadFromConfig("../Tower Defense/Tower defense.json");

	if (!_project.getLevelReader().loadLevel(_project.getLevelList()[0]))
		assert(false && "Failed to load level list.");

	// Seems pointless but the project's ledger path will be in a file just not done yet.
	_project._ledgerPath = "../Tower Defense/Ledger.json";
	_assetLedger._ledgerFilePath = _project._ledgerPath;
	_assetLedger.load(); //eventually do this per project
	//_assetLedger.purge();

	auto num_threads_available = std::thread::hardware_concurrency() - 1;
	_aeonLoader.resizeThreadPool(num_threads_available);	//8

	_shaderManager = ShaderManager(_assetLedger, _renderer.device());
	_textureManager = std::move(TextureManager(_assetLedger, _aeonLoader, _renderer.device()));

	_materialManager = MaterialManager(_assetLedger, _aeonLoader, _shaderManager, _textureManager);

	_skeletonManager = SkeletonManager(_assetLedger, _aeonLoader);
	
	_animationManager = AnimationManager(_assetLedger, _aeonLoader);

	_modelManager = ModelManager(_assetLedger, _aeonLoader, _materialManager, _skeletonManager, _animationManager);



	//auto test1 = _modelManager.getBlocking(9916003768089073041);
	//auto test2 = _modelManager.get(9916003768089073041);

	//_assetManagerLocator.registerManagerForType(EAssetType::SK_MODEL, &_modelManager);
	//_assetManagerLocator.registerManagerForType(EAssetType::MATERIAL, &_materialManager);
	//_assetManagerLocator.registerManagerForType(EAssetType::TEXTURE, &_textureManager);
	//_assetManagerLocator.registerManagerForType(EAssetType::SHADER, &_shaderManager);
	// Another option is this. Wrap them all into one class.
	//OmniAssetManager OAM(
	//	_aeonLoader,
	//	_assetLedger, 
	//	_assetManagerLocator);
	//OAM.request<SkModel>(9916003768089073041);

	_levelMan = new LevelManager(*this);

	return true;
}



void Engine::start()
{
	_clock.reset();

	MSG msg = {};
	bool done = false;

	// Loop until there is a quit message from the window or the user.
	while(!done)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			_clock.tick();
			_fpsCounter.tickAccurate(_clock.deltaTime());
			done = !tick(_clock.deltaTime());	// Otherwise do the frame processing.
		}
	}
}



bool Engine::tick(float dTime)
{
	if (_inputManager.isKeyDown(VK_ESCAPE))
		return false;

	if (!_renderer.frame(dTime))
		return false;

	_levelMan->handleInput(*this, dTime);

	_levelMan->updateAndDrawCurrent(_renderer.rc);

	//_colEngine.update(); Old stuff, has some nice code in there though, need to pull it out and refactor it into something useful

	_inputManager.update();

	return true;
}



void Engine::shutDown()
{
	ShowCursor(true);

	if (FULL_SCREEN)
		ChangeDisplaySettings(NULL, 0);
}



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Engine::HandleWindowInput(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
		return true;

	static uint16_t newSize[2]{0, 0};

	switch (message)
	{
		case WM_KEYDOWN:
		{
			_inputManager.setKeyPressed((unsigned int)wparam);
			break;
		}
		case WM_KEYUP:
		{
			_inputManager.setKeyReleased((unsigned int)wparam);
			break;
		}
		case WM_INPUT:
		{
			UINT dwSize{};

			GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			
			if (dwSize > 0)
			{
				LPBYTE lpb = new BYTE[dwSize];

				if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
					OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

				RAWINPUT* raw = (RAWINPUT*)lpb;

				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					_inputManager.setRelativeXY((short)(raw->data.mouse.lLastX), (short)(raw->data.mouse.lLastY));
				}

				delete[] lpb;
			}		
			break;
		}

		case WM_SIZE:
		{
			//auto flag = wparam;
			//newSize[0] = LOWORD(lparam);	// width of client area
			//newSize[1] = HIWORD(lparam);	// height of client area
			break;
		}

		case WM_EXITSIZEMOVE:
		{
			//_renderer.resize(newSize[0], newSize[1]);
			break;
		}
		
		//mouse
		{
		case WM_ACTIVATEAPP:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
			DirectX::Mouse::ProcessMessage(message, wparam, lparam);
			break;
		}
		default:
		{
			return DefWindowProc(hwnd, message, wparam, lparam);
		}
	}

	return 0;
}