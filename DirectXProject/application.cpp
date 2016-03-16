
#include "application.h"
#include <fstream>


Application::Application()
:m_hMainHandle(NULL)
,m_angle(0.0f)
{

}

Application::~Application()
{

}

HRESULT Application::Init(HINSTANCE hInstance, bool windowed, WNDPROC wndProc)
{
	if(HRESULT ret = InitWindows(hInstance, wndProc) != S_OK)
	{
		return ret;
	}

	if(HRESULT ret = InitD3D(windowed) != S_OK)
	{
		return ret;
	}

	return S_OK;


	



}

HRESULT Application::InitWindows(HINSTANCE hInstance, WNDPROC wndProc)
{
	g_debug << "application init! \n";

	// create window class
	WNDCLASS wc;
	FMEMZERO(&wc, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = wndProc;
	wc.lpszClassName = "D3DWND";

	RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };

	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	RegisterClass(&wc);

	m_hMainHandle = CreateWindow(
		"D3DWND",
		"Character Animation with Direct3D: Example 2.1", //Title
		WS_OVERLAPPEDWINDOW,	//Style
		0, // X
		0, // Y
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		0);

	SetCursor(NULL);
	ShowWindow(m_hMainHandle, SW_SHOW);
	UpdateWindow(m_hMainHandle);

	return S_OK;
}

HRESULT Application::InitD3D(bool windowed)
{
	//Create IDirect3D9 Interface
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if(d3d9 == NULL)
	{
		g_debug << "Direct3DCreate9() - FAILED \n";
		return E_FAIL;
	}


	//Check that the Device supports what we need from it
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

	//Check vertex & pixelshader versions
	if(caps.VertexShaderVersion < D3DVS_VERSION(2, 0) || caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
	{
		g_debug << "Warning - Your graphic card does not support vertex and pixelshaders version 2.0 \n";
	}


	// Set D3DPRESENT_PARAMETERS
	
	m_d3dParam.BackBufferWidth    = WINDOW_WIDTH;  // 后备缓冲区宽度
	m_d3dParam.BackBufferHeight   = WINDOW_HEIGHT; // 后备缓冲区高度
	m_d3dParam.BackBufferFormat   = D3DFMT_A8R8G8B8; // 后备缓冲区颜色模式
	m_d3dParam.BackBufferCount    = 2;             // 后备缓冲区个数
	m_d3dParam.MultiSampleType    = D3DMULTISAMPLE_NONE; //多重采样类别，用于图形反锯齿。
	m_d3dParam.MultiSampleQuality = 0;
	m_d3dParam.SwapEffect         = D3DSWAPEFFECT_DISCARD; // 系统如何将后台缓冲区的内容复制到前台缓冲区, 清除后台缓冲区
	m_d3dParam.hDeviceWindow      = m_hMainHandle; // 指定进行渲染的窗口
	m_d3dParam.Windowed           = windowed;  // 指定窗口模式。True = 窗口模式；False = 全屏模式
	m_d3dParam.EnableAutoDepthStencil = TRUE;  // 是否需要directx自动管理深度缓存 
	m_d3dParam.AutoDepthStencilFormat = D3DFMT_D24S8; // 深度缓冲的格式:指定一个32位的深度/模板缓冲区，其中24位用于深度缓冲区，8位用于模板缓冲区
	m_d3dParam.Flags                  = 0; // 其他的附加特性标志（通常指定为0或NULL）
	
	// D3DPRESENT_RATE_DEFAULT：表示使用默认刷新率，即与屏幕刷新率相同。
	// D3DPRESENT_RATE_UNLIMITED：表示图形绘制结束后立刻将内容复制到前台缓冲区。
	m_d3dParam.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	// 前后台缓冲区交换方式
	m_d3dParam.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;


	int vp = 0;
	if(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	int hrResult = 0;
	HRESULT hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		m_hMainHandle,
		vp,
		&m_d3dParam,
		&g_pDevice);
	if(FAILED(hr))
	{
		switch(hr) 
		{
			case D3DERR_DEVICELOST:
			{
				hrResult = 1;
				break;
			};
			case D3DERR_INVALIDCALL:
			{
				hrResult = 2;
				break;
			};
			case D3DERR_NOTAVAILABLE:
			{
				hrResult = 3;
				break;
			};
			case D3DERR_OUTOFVIDEOMEMORY:
			{
				hrResult = 4;
				break;
			};
			default:
				hrResult = 5;
				break;
				
		}
		//PEXCEPTION_POINTERS ExceptionPointers = (PEXCEPTION_POINTERS)GetExceptionInformation();
		g_debug<<"Failed to create IDirect3DDevice9 \n";
		hrResult;
		return E_FAIL;
	}


	//Release IDirect3D9 interface
	d3d9->Release();

	// load application specific resource here 
	D3DXCreateFont(
		g_pDevice, 20, 0, FW_BOLD, 1, false,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pFont
		);

	//Create Sprite
	D3DXCreateSprite(g_pDevice, &g_pSprite);

	// load effect 
	ID3DXBuffer *pErrorMsgs = NULL;

	HRESULT hRes = D3DXCreateEffectFromFile(
		g_pDevice, "resources/fx/lighting.fx", 
		NULL, NULL, D3DXSHADER_DEBUG, NULL, 
		&g_pEffect, &pErrorMsgs );

	if(FAILED(hRes) && (pErrorMsgs != NULL))		//Failed to create Effect
	{
		MessageBox(NULL, (LPCTSTR )pErrorMsgs->GetBufferPointer(), "Effect Error", MB_OK);
		return E_FAIL;
	}

	//m_soldier.Load("resources/meshes/soldier.x");
	m_drone.Load("resources/meshes/soldier.x");

	m_bDeviceLost = false;

	return S_OK;
}

void Application::DeviceLost()
{
	g_pFont->OnLostDevice();
	g_pSprite->OnLostDevice();
	g_pEffect->OnLostDevice();
	m_bDeviceLost = true;
}

void Application::DeviceGain()
{
	g_pDevice->Reset(&m_d3dParam);
	g_pFont->OnResetDevice();
	g_pSprite->OnResetDevice();
	g_pEffect->OnResetDevice();
	m_bDeviceLost = false;
}

void Application::Update(float deltaTime)
{
	HRESULT coop = g_pDevice->TestCooperativeLevel();

	if (coop != D3D_OK) 
	{
		switch(coop)
		{
		case D3DERR_DEVICELOST:
		{
			if(m_bDeviceLost == false)
			{
				DeviceLost();
			}
		};
		break;
		case D3DERR_DEVICENOTRESET:
		{
			if (m_bDeviceLost==true)
			{
				DeviceGain();
			} 
		};
		break;
		default:
			break;
		}
		Sleep(100);
		return;
	}
	

	// camera rotation 
	m_angle += deltaTime;

	//Keyboard input
	if(KeyDown(VK_ESCAPE))
	{
		Quit();
	}

	if ( KeyDown(VK_RETURN) && KeyDown(18) ) // ALT + RETURN
	{
		//Switch between windowed mode and fullscreen mode
		m_d3dParam.Windowed = !m_d3dParam.Windowed;
		DeviceLost();
		DeviceGain();

		if (m_d3dParam.Windowed) 
		{
			RECT rc = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
			SetWindowPos(m_hMainHandle, HWND_NOTOPMOST, 0, 0, rc.right-rc.left, rc.top-rc.bottom, SWP_SHOWWINDOW);
			UpdateWindow(m_hMainHandle);
		}
	}

}

void Application::CleanUp()
{
	//Release all resources here...
	if(g_pSprite != NULL)	g_pSprite->Release();
	if(g_pFont   != NULL)   g_pFont->Release();
	if(g_pDevice != NULL)	g_pDevice->Release();
	if(g_pEffect != NULL)	g_pEffect->Release();

	g_debug << "Application Terminated \n";
}

void Application::Quit()
{
	DestroyWindow(m_hMainHandle);
	PostQuitMessage(0);
}

void Application::Render()
{
	if ( m_bDeviceLost == true ) 
	{
		return ;
	}

	//Create Matrices
	D3DXMATRIX identity, world, view, proj, shadow;
	D3DXMatrixIdentity(&identity);
	D3DXMatrixRotationY(&world, m_angle);
	//D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0.0f, 1.5f, -3.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	//D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 1000.0f);
	D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(cos(m_angle) * 3.0f, 1.5f, sin(m_angle) * 3.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 1000.0f);
	D3DXPLANE ground(0.0f, 1.0f, 0.0f, 0.0f);
	D3DXVECTOR4 lightPos(-20.0f, 75.0f, -120.0f, 0.0f);
	D3DXMatrixShadow(&shadow, &lightPos, &ground);


	g_pDevice->SetTransform(D3DTS_WORLD, &identity);
	g_pDevice->SetTransform(D3DTS_VIEW, &view);
	g_pDevice->SetTransform(D3DTS_PROJECTION, &proj);

	// 清空目标缓存和深度缓存（用0xffffffff, 1.0f）
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);

	if (SUCCEEDED(g_pDevice->BeginScene())) 
	{
	
		// render soldier
		{
			g_pEffect->SetMatrix("matW", &identity);
			g_pEffect->SetMatrix("matVP", &(view * proj));
			g_pEffect->SetVector("lightPos", &lightPos);

			D3DXHANDLE hTech = g_pEffect->GetTechniqueByName("Lighting");
			g_pEffect->SetTechnique(hTech);
			g_pEffect->Begin(NULL, NULL);
			g_pEffect->BeginPass(0);

			m_drone.Render(NULL);
			

			g_pEffect->EndPass();
			g_pEffect->End();
		} 
		/*
		// render shadow
		{
			g_pEffect->SetMatrix("matW", &identity);
			D3DXHANDLE hTech = g_pEffect->GetTechniqueByName("Shadow");
			g_pEffect->SetTechnique(hTech);
			g_pEffect->Begin(NULL, NULL);
			g_pEffect->BeginPass(0);

			m_drone.Render(NULL);

			g_pEffect->EndPass();
			g_pEffect->End();
		}
		*/
		
		if(KeyDown(VK_SPACE))
		{
			g_pDevice->Clear(0L, NULL, D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L);
			m_drone.RenderSkeleton(NULL, NULL, identity);
		}

		RECT rc = { 10, 10, 0, 0 };
		g_pFont->DrawText(NULL, "Press Space to Toggle Skeleton", -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP, 0x66000000);


		//m_drone.RenderSkeleton(NULL, NULL, world);
		// End the scene.
		g_pDevice->EndScene();
		g_pDevice->Present(0, 0, 0, 0);
	}
}





