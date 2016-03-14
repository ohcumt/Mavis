

#pragma once 
#include <windows.h>
#include "utility.h"
#include "mesh.h"



class Application
{
public:
	Application();
	~Application();

	HRESULT Init(HINSTANCE hInstance, bool windowed, WNDPROC wndProc);
	void DeviceLost();
	void DeviceGain();
	void Update(float deltaTime);
	void Render();
	void CleanUp();
	void Quit();

private:
	HWND m_hMainHandle;
	D3DPRESENT_PARAMETERS m_d3dParam;
	Mesh m_soldier;
	bool m_bDeviceLost;
	float m_angle;


private: 
	HRESULT InitWindows(HINSTANCE hInstance, WNDPROC wndProc);

	HRESULT InitD3D(bool windowed);


};





