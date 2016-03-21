#include <d3d9.h>
#include <strsafe.h>

#include "application.h"

//The Window procedure function
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//User specified events
	switch(msg)
	{
	case WM_CREATE:
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	//Default events
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE preInstance,
	PSTR cmdLine,
	int showCmd
	)
{
	// create a new application object
	Application app;

	// init app
	if(FAILED(app.Init(hInstance, true, WndProc)))
	{
		return 0;
	}


	MSG msg;
	memset(&msg, 0, sizeof(MSG));

	// keep track of the time 
	DWORD startTime = GetTickCount();

	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			//If there's a message, deal with it and send it onwards
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else	//Otherwise update the game
		{
			//Calculate the delta time
			DWORD t = GetTickCount();
			float deltaTime = (t - startTime)*0.001f;

			//Update the application
			app.Update(deltaTime);

			//Render the application
			app.Render();

			startTime = t;
		}
	}

	//Release all resources
	app.CleanUp();

	//... and Quit!
	return (int)msg.wParam;

}