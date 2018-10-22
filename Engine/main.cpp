#include "Systems.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow){

	Systems* Sys;
	
	// Create the system object.
	Sys = new Systems;
	if(!Sys){
		std::cout << "SystemClass constructor failed." << std::endl;
		return 1;
	}

	// Initialize and run the system object.
	if(!Sys->Initialize()){
		std::cout << "System could not be initialized." << std::endl;
		return 2;
	}

	Sys->Run();

	// Shutdown and release the system object.
	Sys->Shutdown();
	delete Sys;
	Sys = 0;

	return 0;
}