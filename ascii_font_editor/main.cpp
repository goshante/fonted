#include <iostream>
#include "Application.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	try
	{ 
		Application app;
		while (app.ProcessEventLoop())
			Sleep(1);
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Exception", MB_OK | MB_ICONERROR);
	}
	return 0;
}