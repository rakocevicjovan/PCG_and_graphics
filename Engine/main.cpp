#include "pch.h"
#include "Engine.h"
//#include "Editor.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	if (!SetProcessDPIAware())
	{
		assert(false && "SetProcessDPIAware() failed!");
	}

	// For debugging with console enable this
	//AllocConsole();
	//freopen("CONOUT$", "w", stdout);
	//freopen("CONOUT$", "w", stderr);

	Engine engine;
	engine.initialize();
	engine.start();

	//Editor editor(engine);
	//editor.start();

	return 0;
}