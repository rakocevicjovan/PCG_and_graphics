#include "Engine.h"
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	std::unique_ptr<Engine> engine;
	
	engine = std::make_unique<Engine>();

	if(!engine)
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