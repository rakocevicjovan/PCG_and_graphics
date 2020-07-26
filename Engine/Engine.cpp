#include "Engine.h"
#include "GUI.h"
#include <string>
#include <Mouse.h>

#include "ShaderGenerator.h"


Engine::Engine() :
	_scrWidth(0), 
	_scrHeight(0),
	_threadPool(std::thread::hardware_concurrency() - 1)
{}


Engine::~Engine() {}


bool Engine::Initialize()
{
	// Testing grounds...

	//FBXLoader loader;
	//loader.init();
	//loader.parse("C:\\Users\\Senpai\\source\\repos\\PCG_and_graphics_stale_memes\\Models\\Animated\\ArmyPilot\\ArmyPilot.fbx");

	//ShaderGenerator::preprocessAllPermutations(
		//L"ShGen\\VS_proto.hlsl", "ShGen\\GeneratedVS\\vs_");

	InitializeWindows(_scrWidth, _scrHeight);

	if (!_D3D.Initialize(_windowWidth, _windowHeight, false, _hwnd, FULL_SCREEN))
	{
		MessageBox(_hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	_device = _D3D.GetDevice();
	_deviceContext = _D3D.GetDeviceContext();

	_inputManager.initialize(_hwnd);
	_defController = Controller(&_inputManager);
	_inputManager.registerController(&_defController);

	if (!_renderer.initialize(_windowWidth, _windowHeight, _D3D))
	{
		_renderer._cam._controller = &_defController;
		MessageBox(_hwnd, L"Could not initialize Renderer.", L"Error", MB_OK);
		return false;
	}


	//loads in the td game and first level for now... cba going through the selection each time
	_resMan.init(_device);

	_shaderCompiler.init(&_hwnd, _device);
	_shaderCache.init(&_shaderCompiler);
	_matCache.init(&_shaderCache, &_resMan);
	
	//_colEngine.init();		//_colEngine.registerController(_defController);
	_renderer._cam._controller = &_defController;

	GUI::initDxWin32(_hwnd, _device, _deviceContext);

	// Loads the project configuration data into the project loader, as well as a list of levels associated to the project
	_project.loadFromConfig("C:/Users/Senpai/source/repos/PCG_and_graphics_stale_memes/Tower Defense/Tower defense.json");
	
	if (!_project.getLevelReader().loadLevel(_project.getLevelList()[0]))
		assert(false && "Failed to load level list.");

	// Seems pointless but the project's ledger path will be in a file just not done yet.
	_project._ledgerPath = "C:/Users/Senpai/source/repos/PCG_and_graphics_stale_memes/Tower Defense/Ledger.json";
	_resMan._assetLedger._ledgerFilePath = _project._ledgerPath;

	_resMan._assetLedger.purge();
	_resMan._assetLedger.load();

	_levelMan = new LevelManager(*this);

	return true;
}



void Engine::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int _posX, _posY;

	ApplicationHandle = this;	// Get an external pointer to this object.	

	_hinstance = GetModuleHandle(NULL);	// Get the instance of this application.

	_applicationName = L"Aeolian";	// Give the application a name.

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = _hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = _applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		_windowWidth = screenWidth;
		_windowHeight = screenHeight;

		// Set the position of the window to the top left corner.
		_posX = _posY = 0;
	}
	else
	{
		_posX = (GetSystemMetrics(SM_CXSCREEN) - _windowWidth) / 2;
		_posY = (GetSystemMetrics(SM_CYSCREEN) - _windowHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	_hwnd = CreateWindowEx(WS_EX_APPWINDOW, _applicationName, _applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		_posX, _posY, _windowWidth, _windowHeight, NULL, NULL, _hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(_hwnd, SW_SHOW);
	SetForegroundWindow(_hwnd);
	SetFocus(_hwnd);

	// Show or hide the mouse cursor.
	ShowCursor(false);

	return;
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

	DestroyWindow(_hwnd);
	_hwnd = NULL;

	UnregisterClass(_applicationName, _hinstance);
	_hinstance = NULL;

	ApplicationHandle = NULL;
}



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	if(ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam))
		return true;

	switch(umessage)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}

		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}



LRESULT CALLBACK Engine::MessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
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