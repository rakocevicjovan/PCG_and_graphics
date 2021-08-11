#include "pch.h"
#include "Engine.h"
//#include "Editor.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	if (!SetProcessDPIAware())
	{
		assert(false && "Couldn't make application dpi aware!");
	}

	Engine engine;
	engine.initialize();
	engine.start();

	//Editor editor(engine);
	//editor.start();

	return 0;
}