#include "Systems.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	Systems* Sys;
	
	Sys = new Systems;
	if(!Sys)
	{
		std::cout << "SystemClass constructor failed." << std::endl;
		return 1;
	}

	if(!Sys->Initialize())
	{
		std::cout << "System could not be initialized." << std::endl;
		return 2;
	}

	Sys->Run();

	Sys->Shutdown();
	delete Sys;
	Sys = nullptr;

	return 0;
}