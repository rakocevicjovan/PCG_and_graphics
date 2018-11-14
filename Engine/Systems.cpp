#include "Systems.h"
#include <string>

Systems::Systems() : screenWidth(0), screenHeight(0), _renderer(0), _audio(0){

}

Systems::~Systems(){
}


bool Systems::Initialize(){

	bool result;

	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create the input object.  This object will be used to handle reading the keyboard input from the user.

	// Initialize the input object.
	_input.Initialize();

	// Create the graphics object.  This object will handle rendering all the graphics for this application.
	_renderer = new Renderer;
	if(!_renderer){
		return false;
	}

	// Initialize the graphics object.
	result = _renderer->Initialize(windowWidth, windowHeight, m_hwnd, _input);
	if(!result){
		return false;
	}

	//_input.mouse->SetWindow(m_hwnd);
	//_input.mouse->SetMode(DirectX::Mouse::Mode::MODE_RELATIVE);
	//@TODO init audio

	return true;
}



void Systems::InitializeWindows(int& screenWidth, int& screenHeight){

	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = L"Aeolipile";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
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

		windowWidth = screenWidth;
		windowHeight = screenHeight;

		// Set the position of the window to the top left corner.
		posX = posY = 0;

	}else{

		windowWidth = 1600;
		windowHeight = 900;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
	}

	//@TODO MIGHT NOT NEED THIS/////
	midWindow.x = windowWidth / 2;
	midWindow.y = windowHeight / 2;
	////////////////////////////////

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, windowWidth, windowHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);

	return;
}



void Systems::Run()
{
	MSG msg;
	bool done = false;

	gc.Reset();
	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));
	
	// Loop until there is a quit message from the window or the user.
	while(!done){
		// Handle the windows messages.
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT){
			done = true;
		}
		else{
			gc.Tick();
			done = !Frame(gc.DeltaTime());	// Otherwise do the frame processing.
		}
	}
}


bool Systems::Frame(float dTime){

	// Check if the user pressed escape and wants to exit the application.
	if(_input.IsKeyDown(VK_ESCAPE))
		return false;

	bool res = _renderer->Frame(dTime);// Do the frame processing for the graphics object.

	//reset input so rotations don't keep happening
	_input.SetXY(0, 0);
	return res;
}






void Systems::Shutdown()
{
	// Release the graphics object.
	if (_renderer)
	{
		_renderer->Shutdown();
		delete _renderer;
		_renderer = 0;
	}

	// Release the input object.
	//no need, it's not a pointer any more

	//@TODO this is where I'd delete my audio system... if I had one

	// Shutdown the window.
	ShutdownWindows();

	return;
}


void Systems::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(FULL_SCREEN){
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam){
	switch(umessage){
		case WM_DESTROY:{
			PostQuitMessage(0);
			return 0;
		}

		case WM_CLOSE:{
			PostQuitMessage(0);		
			return 0;
		}

		default:{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}

LRESULT CALLBACK Systems::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {


	switch (umsg) {

		case WM_KEYDOWN:
		{
			_input.KeyDown((unsigned int)wparam);
			break;
		}
		case WM_KEYUP:
		{
			_input.KeyUp((unsigned int)wparam);
			break;
		}
		/*
		case WM_MOUSEMOVE:{
			_input.SetXY(MAKEPOINTS(lparam).x, MAKEPOINTS(lparam).y);
			//_input.mouse->ProcessMessage(umsg, wparam, lparam);
			break;
		}
		*/

		//DO NOT DELETE THIS CODE - IT WORKS AND WILL PROBABLY BE USEFUL
		case WM_INPUT:
		{
			UINT dwSize;

			GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			
			if (dwSize > 0) {

				LPBYTE lpb = new BYTE[dwSize];

				if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
					OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

				RAWINPUT* raw = (RAWINPUT*)lpb;

				/*if (raw->header.dwType == RIM_TYPEKEYBOARD){}*/
				if (raw->header.dwType == RIM_TYPEMOUSE) {
					_input.SetXY((short)(raw->data.mouse.lLastX), (short)(raw->data.mouse.lLastY));
					//std::string wat = "RAW INPUT RECEIVED X:" + std::to_string(raw->data.mouse.lLastX) + "; Y: " + std::to_string(raw->data.mouse.lLastY) + "\n";
					//OutputDebugStringA(wat.c_str());
				}

				delete[] lpb;
			}		
			break;
		}
		default: {
			return DefWindowProc(hwnd, umsg, wparam, lparam);
		}
	}
	return 0;
}