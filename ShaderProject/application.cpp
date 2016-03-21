
#include "application.h"
#include <fstream>


Application::Application()
:m_hMainHandle(NULL)
, m_angle(0.0f)
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

	m_d3dParam.BackBufferWidth = WINDOW_WIDTH;  // �󱸻��������
	m_d3dParam.BackBufferHeight = WINDOW_HEIGHT; // �󱸻������߶�
	m_d3dParam.BackBufferFormat = D3DFMT_A8R8G8B8; // �󱸻�������ɫģʽ
	m_d3dParam.BackBufferCount = 2;             // �󱸻���������
	m_d3dParam.MultiSampleType = D3DMULTISAMPLE_NONE; //���ز����������ͼ�η���ݡ�
	m_d3dParam.MultiSampleQuality = 0;
	m_d3dParam.SwapEffect = D3DSWAPEFFECT_DISCARD; // ϵͳ��ν���̨�����������ݸ��Ƶ�ǰ̨������, �����̨������
	m_d3dParam.hDeviceWindow = m_hMainHandle; // ָ��������Ⱦ�Ĵ���
	m_d3dParam.Windowed = windowed;  // ָ������ģʽ��True = ����ģʽ��False = ȫ��ģʽ
	m_d3dParam.EnableAutoDepthStencil = TRUE;  // �Ƿ���Ҫdirectx�Զ�������Ȼ��� 
	m_d3dParam.AutoDepthStencilFormat = D3DFMT_D24S8; // ��Ȼ���ĸ�ʽ:ָ��һ��32λ�����/ģ�建����������24λ������Ȼ�������8λ����ģ�建����
	m_d3dParam.Flags = 0; // �����ĸ������Ա�־��ͨ��ָ��Ϊ0��NULL��

	// D3DPRESENT_RATE_DEFAULT����ʾʹ��Ĭ��ˢ���ʣ�������Ļˢ������ͬ��
	// D3DPRESENT_RATE_UNLIMITED����ʾͼ�λ��ƽ��������̽����ݸ��Ƶ�ǰ̨��������
	m_d3dParam.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	// ǰ��̨������������ʽ
	m_d3dParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;


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
		g_debug << "Failed to create IDirect3DDevice9 \n";
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


	m_bDeviceLost = false;

	return S_OK;
}

void Application::DeviceLost()
{
	g_pFont->OnLostDevice();
	g_pSprite->OnLostDevice();
	m_bDeviceLost = true;
}

void Application::DeviceGain()
{
	g_pDevice->Reset(&m_d3dParam);
	g_pFont->OnResetDevice();
	g_pSprite->OnResetDevice();
	m_bDeviceLost = false;
}

void Application::Update(float deltaTime)
{
	HRESULT coop = g_pDevice->TestCooperativeLevel();

	if(coop != D3D_OK)
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
			if(m_bDeviceLost == true)
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

	if(KeyDown(VK_RETURN) && KeyDown(18)) // ALT + RETURN
	{
		//Switch between windowed mode and fullscreen mode
		m_d3dParam.Windowed = !m_d3dParam.Windowed;
		DeviceLost();
		DeviceGain();

		if(m_d3dParam.Windowed)
		{
			RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
			SetWindowPos(m_hMainHandle, HWND_NOTOPMOST, 0, 0, rc.right - rc.left, rc.top - rc.bottom, SWP_SHOWWINDOW);
			UpdateWindow(m_hMainHandle);
		}
	}

}

void Application::CleanUp()
{
	//Release all resources here...
	if(g_pSprite != NULL)	g_pSprite->Release();
	if(g_pFont != NULL)   g_pFont->Release();
	if(g_pDevice != NULL)	g_pDevice->Release();

	g_debug << "Application Terminated \n";
}

void Application::Quit()
{
	DestroyWindow(m_hMainHandle);
	PostQuitMessage(0);
}

void Application::Render()
{
	if(m_bDeviceLost == true)
	{
		return;
	}

	D3DXMATRIX identify, shadow;
	D3DXMatrixIdentity(&identify);

	D3DXPLANE ground(0.0f, 1.0f, 0.0f, 0.0f);
	D3DXVECTOR4 lightPos(-20.0f, 75.0f, -120.0f, 0.0f);

	// ����ƽ����Ӱ�任����
	D3DXMatrixShadow(&shadow, &lightPos, &ground);



	D3DXMATRIX view, proj, world;
	D3DXMatrixIdentity(&world);


	D3DXVECTOR3 position(cos(m_angle) * 2.0f, 2.0f, sin(m_angle) * 2.0f); //camera����������ϵ�е�λ������
	D3DXVECTOR3 target(0.0f, 1.0f, 0.0f);//target��camera�ĳ���������������ԭ�㣬Ҳ�����������۲��
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);//�������ϵķ���һ����[0,1,0]
	// ���� ����->��ͼ ����ϵ�ı任����
	D3DXMatrixLookAtLH(&view, &position, &target, &up);



	float fovy = D3DX_PI / 4.0f; // �ӽ�
	float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT; // ��߱�
	float zn = 0.1f;    // ���ý���
	float zf = 100.0f;  // Զ�ý���
	// ����任����  --> ��3D���������任��
	// (1,1,0) (-1,1,0) (-1,-1,0) (1,-1,0) (1,1,1) (-1,1,1) (-1,-1,1) (1,-1,1)���С������
	D3DXMatrixPerspectiveFovLH(&proj, fovy, aspect, zn, zf);

	//  ���ñ任����, ʵ�ʱ任��ʱ��,�ȵõ��任����,�ٰ�����е�������Ա任����
	//Set transformation matrices
	g_pDevice->SetTransform(D3DTS_WORLD, &world);
	g_pDevice->SetTransform(D3DTS_VIEW, &view);
	g_pDevice->SetTransform(D3DTS_PROJECTION, &proj);


	// ���Ŀ�껺�����Ȼ��棨��0xffffffff, 1.0f��
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

	if(SUCCEEDED(g_pDevice->BeginScene()))
	{

	}

	// End the scene.
	g_pDevice->EndScene();
	g_pDevice->Present(0, 0, 0, 0);
}





