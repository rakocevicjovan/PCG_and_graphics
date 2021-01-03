#include "Engine.h"
#include "GUI.h"
#include <string>
#include <Mouse.h>

#include "ShaderGenerator.h"


Engine::Engine() :
	_scrWidth(GetSystemMetrics(SM_CXSCREEN)),
	_scrHeight(GetSystemMetrics(SM_CYSCREEN)),
	_threadPool(std::thread::hardware_concurrency() - 1)
{}


Engine::~Engine()
{
	Shutdown();
}


bool Engine::Initialize()
{
	// Testing grounds...

	//FBXLoader loader;
	//loader.init();
	//loader.parse("C:\\Users\\Senpai\\source\\repos\\PCG_and_graphics_stale_memes\\Models\\Animated\\ArmyPilot\\ArmyPilot.fbx");

	//ShaderGenerator::preprocessAllPermutations(L"ShGen\\VS_proto.hlsl", "ShGen\\GeneratedVS\\vs_");
	
	_engineWindow.create("Aeolian engine", this, _windowWidth, _windowHeight,
		Window<Engine>::CreationFlags::SHOW_WINDOW |
		Window<Engine>::CreationFlags::START_FOCUSED |
		Window<Engine>::CreationFlags::START_FOREGROUND);

	if (!_D3D.Initialize(_windowWidth, _windowHeight, false, _engineWindow._hwnd, FULL_SCREEN))
	{
		MessageBox(_engineWindow._hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	_device = _D3D.GetDevice();
	_deviceContext = _D3D.GetDeviceContext();

	_inputManager.initialize(_engineWindow._hwnd);
	_defController = Controller(&_inputManager);
	_inputManager.registerController(&_defController);

	if (!_renderer.initialize(_windowWidth, _windowHeight, _D3D))
	{
		_renderer._cam._controller = &_defController;
		MessageBox(_engineWindow._hwnd, L"Could not initialize Renderer.", L"Error", MB_OK);
		return false;
	}

	_resMan.init(_device);

	_shaderCompiler.init(_device);
	_shaderCache.init(&_shaderCompiler);
	_matCache.init(&_shaderCache, &_resMan);
	
	//_colEngine.init();		//_colEngine.registerController(_defController);
	_renderer._cam._controller = &_defController;

	GUI::initDxWin32(_engineWindow._hwnd, _device, _deviceContext);

	// Loads the project configuration data into the project loader, as well as a list of levels associated to the project
	_project.loadFromConfig("../Tower Defense/Tower defense.json");
	
	if (!_project.getLevelReader().loadLevel(_project.getLevelList()[0]))
		assert(false && "Failed to load level list.");

	// Seems pointless but the project's ledger path will be in a file just not done yet.
	_project._ledgerPath = "../Tower Defense/Ledger.json";
	_resMan._assetLedger._ledgerFilePath = _project._ledgerPath;

	_resMan._assetLedger.load();
	_resMan._assetLedger.purge();

	_levelMan = new LevelManager(*this);

	return true;
}



void Engine::Run()
{
	_clock.Reset();

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
			_clock.Tick();
			done = !Frame(_clock.DeltaTime());	// Otherwise do the frame processing.
		}
	}
}



bool Engine::Frame(float dTime)
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



void Engine::OutputFPS(float dTime)
{
	std::ostringstream ss;
	ss << "Frame time: " << 1.0f / dTime << "\n";
	std::string s(ss.str());
	OutputDebugStringA(ss.str().c_str());
}



void Engine::Shutdown()
{
	ShowCursor(true);

	if (FULL_SCREEN)
		ChangeDisplaySettings(NULL, 0);

	DestroyWindow(_engineWindow._hwnd);
	_engineWindow._hwnd = NULL;

	UnregisterClass(_applicationName, _hinstance);
	_hinstance = NULL;
}



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Engine::HandleWindowInput(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
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
			UINT dwSize;

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


/* manual mouse querying code... using directxtk now

case WM_ACTIVATEAPP:
		{
			break;
		}
		case WM_MOUSEMOVE:
		{
			break;
		}
		case WM_LBUTTONDOWN:
		{
			_inputManager.mouseLPressed();
			break;
		}
		case WM_LBUTTONUP:
		{
			_inputManager.mouseLReleased();
			break;
		}
		case WM_RBUTTONDOWN:
		{
			_inputManager.mouseRPressed();
			break;
		}
		case WM_RBUTTONUP:
		{
			_inputManager.mouseRReleased();
			break;
		}
		case WM_MBUTTONDOWN:
		{
			break;
		}
		case WM_MBUTTONUP:
		{
			break;
		}
		case WM_MOUSEWHEEL:
		{
			break;
		}
		case WM_XBUTTONDOWN:
		{
			break;
		}
		case WM_XBUTTONUP:
		{
			break;
		}

*/