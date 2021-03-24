#include "pch.h"
#include "Engine.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	if (!SetProcessDPIAware())
	{
		assert(false && "Couldn't make application dpi aware!");
	}

	auto engine = std::make_unique<Engine>();

	if(!engine.get())
	{
		std::cout << "Engine constructor failed." << std::endl;
		return 1;
	}

	if(!engine->Initialize())
	{
		std::cout << "Engine could not be initialized." << std::endl;
		return 2;
	}

	engine->Run();

	return 0;
}