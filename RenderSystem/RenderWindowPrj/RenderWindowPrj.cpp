// RenderWindowPrj.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RenderWindowPrj.h"
#include "RenderSystem.h"
#include "OpenGLES\EGL\egl.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND g_hWnd;
EGLContext g_glesContext;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
BOOL bHasInit = FALSE;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				Render();
bool				CreateContext(HWND hWnd);

bool CreateContext(HWND hWnd)
{
	EGLConfig configs[10];

	EGLint matchingConfigs;

	//const EGLint configAttribs[] =
	EGLint configAttribs[] =
	{
		EGL_SURFACE_TYPE,	EGL_WINDOW_BIT,
		EGL_BUFFER_SIZE,	24,
		EGL_DEPTH_SIZE,		16,
		EGL_SAMPLE_BUFFERS, 1,
		EGL_SAMPLES,        4,
		EGL_NONE
	};

	HDC hDeviceContext = GetDC(hWnd);
	if (!hDeviceContext)
	{
		OutputDebugString(_T("Unable to create rendering context"));
		return false;
	}

	EGLDisplay glesDisplay = eglGetDisplay(hDeviceContext);

	if (!eglInitialize(glesDisplay, NULL, NULL))
	{
		return false;
	}

	configAttribs[3] = 32;

	if (!eglChooseConfig(glesDisplay, configAttribs, &configs[0], 10, &matchingConfigs))
	{
		return false;
	}

	if (matchingConfigs < 1)
	{
		return false;
	}

	EGLSurface glesSurface = eglCreateWindowSurface(glesDisplay, configs[0], hWnd, NULL);
	if (!glesSurface)
	{
		return false;
	}

	EGLContext glesContext = eglCreateContext(glesDisplay, configs[0], NULL, NULL);
	if (!glesContext)
	{
		return false;
	}

	eglMakeCurrent(glesDisplay, glesSurface, glesSurface, glesContext);
	g_glesContext = glesContext;

	return true;
}

void Render()
{
	CRenderSystem* pRenderSystem = CRenderSystem::GetInstance();
	pRenderSystem->DrawRect(100, 100, 50, 50, 0x00ff0099);

	pRenderSystem->DrawRect(400, 500, 100, 100, 0xFF0000FF);
	pRenderSystem->EndDisplay();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_RENDERWINDOWPRJ, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RENDERWINDOWPRJ));

    MSG msg;
	
	CreateContext(g_hWnd);
	CRenderSystem* pRenderSystem = CRenderSystem::GetInstance();
	stContextParam param;
	param.m_pContext = g_glesContext;
	param.m_Width = 800;
	param.m_Height = 600;
	pRenderSystem->Initialize(&param);
	bHasInit = TRUE;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RENDERWINDOWPRJ));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   g_hWnd = hWnd;

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
   //         PAINTSTRUCT ps;
   //         HDC hdc = BeginPaint(hWnd, &ps);
   //         // TODO: Add any drawing code that uses hdc here...
			//if (bHasInit == TRUE)
			//{
			//	Render();
			//	
			//}
   //         EndPaint(hWnd, &ps);
			////SwapBuffers(hdc);
			//ValidateRect(hWnd, NULL);
			if (bHasInit == TRUE)
			{
				HDC hDC = GetDC(hWnd); //得到当前窗口的设备环境  
				Render();
				SwapBuffers(hDC);
				ValidateRect(hWnd, NULL);
			}
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
