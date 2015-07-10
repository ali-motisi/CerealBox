/*
 * CerealBox - Copyright 2015 Ali Motisi. All rights reserved.
*/

#include <windows.h>
#include "../../common/src/oogame.h"
#include "win32audio.h"

#include <d3d9.h>

#pragma comment (lib, "d3d9.lib")

#define OO_DIRECTXBLIT

typedef struct {
	HWND window;
	HDC windowDC;
	oofloat dt;
	oobool running;
	void * gameMemory;
	OOGameInput gameinput;
	OOGameOutput gameoutput;
	oobool activeWindow;

#ifdef OO_DIRECTXBLIT
	LPDIRECT3D9 d3d; // the pointer to our Direct3D interface
	LPDIRECT3DDEVICE9 d3dDev; // the pointer to the Direct3D device
	D3DPRESENT_PARAMETERS d3dpp;
	IDirect3DSurface9 * d3dSurface;

#else
	HDC srchDC;
	HBITMAP bitmap;
#endif

} oowindata;

static oowindata windata;


void renderGraphics(oowindata* data) {
#ifdef OO_DIRECTXBLIT
	if( !data->activeWindow ) {
		return;
	}
	data->d3dDev->lpVtbl->Clear(data->d3dDev, 0, NULL, D3DCLEAR_TARGET , D3DCOLOR_XRGB(0, 0, 0), 1.f, 0);
	data->d3dDev->lpVtbl->BeginScene(data->d3dDev);
	data->d3dDev->lpVtbl->EndScene(data->d3dDev);

	if( data->d3dSurface ) {
		IDirect3DSurface9 * backBuffer = NULL;
		data->d3dDev->lpVtbl->GetBackBuffer(data->d3dDev,0,0,D3DBACKBUFFER_TYPE_MONO,&backBuffer);

		if(backBuffer) {
			D3DLOCKED_RECT rect;
			data->d3dSurface->lpVtbl->LockRect(data->d3dSurface, &rect, NULL, D3DLOCK_DISCARD);
			memcpy(rect.pBits, data->gameoutput.graphic.buffer, data->gameoutput.graphic.width*data->gameoutput.graphic.height*sizeof(oouint));
			data->d3dSurface->lpVtbl->UnlockRect(data->d3dSurface);

			data->d3dDev->lpVtbl->StretchRect(data->d3dDev,data->d3dSurface,NULL,backBuffer,NULL,D3DTEXF_NONE);
			backBuffer->lpVtbl->Release(backBuffer);
		}
	}

	data->d3dDev->lpVtbl->Present(data->d3dDev, NULL, NULL, NULL, NULL);
#else
	oographic * g = &data->gameoutput.graphic;
	BitBlt(data->windowDC,0,0,g->width,g->height,data->srchDC,0,0,SRCCOPY);
	//StretchBlt(data->desthDC, 0, 0, g->width, g->height, data->srchDC, 0, 0, g->width, g->height, SRCCOPY);
#endif
}

void destroyGraphics(oowindata* data) {
#ifdef OO_DIRECTXBLIT
	if( data->d3dSurface ) {
		data->d3dSurface->lpVtbl->Release(data->d3dSurface);
		data->d3dSurface = NULL;
	}

	if( data->d3dDev ) {
		data->d3dDev->lpVtbl->Release(data->d3dDev);
		data->d3dDev = NULL;
	}

	if( data->d3d ) {
		data->d3d->lpVtbl->Release(data->d3d);
		data->d3d = NULL;
	}

	if(data->gameoutput.graphic.buffer) {
		free(data->gameoutput.graphic.buffer);
	}
	
#else
	DeleteDC(data->srchDC);
	DeleteObject(data->bitmap);
#endif
}



void initGraphics(oowindata* data) {
#ifdef OO_DIRECTXBLIT
	data->d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if( !data->d3d ) {
		return;
	}

	ZeroMemory(&data->d3dpp,sizeof(data->d3dpp));

	data->d3dpp.Windowed = TRUE; // always set to true to avoid problems with the device lost during rendering
	data->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	data->d3dpp.hDeviceWindow = data->window;
	data->d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;

	data->d3dpp.BackBufferWidth = data->gameoutput.graphic.width;
	data->d3dpp.BackBufferHeight = data->gameoutput.graphic.height;

	// create a device class using this information and the info from the d3dpp stuct
	data->d3d->lpVtbl->CreateDevice(data->d3d, D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		data->window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&data->d3dpp,
		&data->d3dDev);

	if( data->d3dDev ) {
		data->d3dDev->lpVtbl->CreateOffscreenPlainSurface(data->d3dDev, data->gameoutput.graphic.width, data->gameoutput.graphic.height, data->d3dpp.BackBufferFormat, 
			D3DPOOL_DEFAULT, &data->d3dSurface, NULL);


		data->gameoutput.graphic.buffer = (oouint *) calloc((size_t)data->gameoutput.graphic.width*data->gameoutput.graphic.height, (size_t)sizeof(oouint));

	}
#else
	HDC hDC;
	BITMAPINFO bitmapinfo;
	oographic * g = &data->gameoutput.graphic;
	hDC = CreateCompatibleDC(NULL);
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biWidth = g->width;
	bitmapinfo.bmiHeader.biHeight = g->height;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 32;
	bitmapinfo.bmiHeader.biCompression = BI_RGB;
	bitmapinfo.bmiHeader.biSizeImage = 0;


	data->bitmap = CreateDIBSection(hDC,&bitmapinfo,DIB_RGB_COLORS,(VOID **)&g->buffer,0,0);
	data->srchDC = CreateCompatibleDC(NULL);
	SelectObject(data->srchDC,data->bitmap);
	DeleteDC(hDC);
#endif
}

void win32ProcessMouseEvents(HWND window, oowindata * data) {
	POINT mouseP;
	GetCursorPos(&mouseP);
	ScreenToClient(window, &mouseP);
	
	data->gameinput.mouse.x = (oofloat) mouseP.x;
	data->gameinput.mouse.y = ((oofloat) mouseP.y);
	data->gameinput.mouse.leftButtonDown = (GetKeyState(VK_LBUTTON) & (1<<15))!=0;
	data->gameinput.mouse.rightButtonDown = (GetKeyState(VK_RBUTTON) & (1<<15))!=0;
}


LRESULT CALLBACK winproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
 
  switch (uMsg) {
  case WM_DESTROY:    
    PostQuitMessage(0);
    break;
  case WM_CREATE:
    break;

#ifdef OO_FULLSCREEN
  case WM_SETCURSOR: {
	  SetCursor(NULL);
  } break;
#endif

  case WM_PAINT: {
	//PAINTSTRUCT PtStr;
    //windata.desthDC = BeginPaint(hWnd,&PtStr);
    //render(&windata);
    //EndPaint(hWnd,&PtStr);
  }
    break;

	case WM_ACTIVATE:
		if(  wParam==WA_INACTIVE ) {
			windata.activeWindow = oofalse;
		}
		else {
			windata.activeWindow = ootrue;
		}
		break;

 case WM_ERASEBKGND:
	 return (LRESULT)1; // this is needed to reduce flickering

  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP: {
	  unsigned int keyCode = (unsigned int)wParam;
	  oobool keyIsDown = ((lParam & (1 << 31)) == 0);
	  oobool keyWasDown = ((lParam & (1 << 30)) != 0);


	  if( keyCode==VK_ESCAPE && keyIsDown ) {
		  windata.running = oofalse;
	  }

	  if( keyIsDown!=keyWasDown ) {
		  if( keyCode>=0 && keyCode<OOGameInput_Keys_Max ) {
			  windata.gameinput.keyboard[keyCode] = keyIsDown;
		  }
	  }
  } break;

  default:
    return DefWindowProc (hWnd, uMsg, wParam, lParam);
    break;
  }
  return 0;
}

void win32GetScreenResolution(ooint * width, ooint * height) {
	*width = GetSystemMetrics(SM_CXSCREEN);
	*height = GetSystemMetrics(SM_CYSCREEN);
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow) {
	wchar_t progname[] = L"OOJam";


	windata.gameoutput.graphic.width = 1920/4;
	windata.gameoutput.graphic.height = 1080/4;

	windata.running = ootrue;
	windata.gameMemory = (void *) calloc((size_t)gameMemorySize(), 1L);

	oouint audioBufferSize = win32AudioBufferSize();
    windata.gameoutput.audio.buffer = (ooshort *) calloc((size_t)audioBufferSize, 1L);
	oouint audioFramesPerSecond = win32AudioFramesPerSecond();

	WNDCLASSEXW winclass;
	ZeroMemory(&winclass, sizeof(WNDCLASSEXW));

	
	MSG msg;

	winclass.cbSize = sizeof(WNDCLASSEXW);
	winclass.style = CS_DBLCLKS;
	winclass.lpfnWndProc = &winproc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hInst;
	winclass.hIcon = LoadIcon(NULL,IDI_WINLOGO);
	winclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	winclass.lpszClassName = progname;


	if(!RegisterClassExW(&winclass)) {
		return 0;
	}

	oouint startX = 100;
	oouint startY = 100;

	DWORD windowStyle =  WS_SYSMENU|WS_CAPTION|WS_BORDER|WS_OVERLAPPED|WS_VISIBLE|WS_MINIMIZEBOX;

#ifdef OO_FULLSCREEN
	windowStyle = WS_EX_TOPMOST | WS_POPUP;
	win32GetScreenResolution(&windata.gameoutput.graphic.width, &windata.gameoutput.graphic.height);
	startX = 0;
	startY = 0;
#endif
	RECT wr = {0, 0, windata.gameoutput.graphic.width, windata.gameoutput.graphic.height};
	AdjustWindowRect(&wr, windowStyle, FALSE);

	windata.window = CreateWindowW(
		progname,
		progname,
		windowStyle,
		startX,
		startY,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInst,
		NULL);

	initGraphics(&windata);
	win32AudioInit(windata.window);

	ShowWindow(windata.window,nCmdShow);
	UpdateWindow(windata.window);

	LARGE_INTEGER timeFrequency;
	QueryPerformanceFrequency(&timeFrequency);

	LARGE_INTEGER lastTime;
	ZeroMemory(&lastTime, sizeof(LARGE_INTEGER) );
	LARGE_INTEGER time;
	windata.gameinput.dt = 1.f/60.f;

	windata.windowDC = GetDC(windata.window);

	ooint monitorRefreshHz = 60;
	oofloat frameMs = 1.f / (oofloat)monitorRefreshHz;
	ooint win32RefreshRate = GetDeviceCaps(windata.windowDC, VREFRESH);
	if( win32RefreshRate>1 ) {
		monitorRefreshHz = win32RefreshRate;
		frameMs = 1.f / (oofloat)monitorRefreshHz;
	}

	while( windata.running ) {
		PeekMessage(&msg,NULL,0,0,PM_REMOVE);
		if(msg.message == WM_QUIT) {
			break;
		}


		TranslateMessage(&msg);
		DispatchMessage(&msg);

		win32ProcessMouseEvents(windata.window, &windata);

		QueryPerformanceCounter(&time);
		if( lastTime.QuadPart>0 ) {
			windata.gameinput.dt = (oofloat)(time.QuadPart - lastTime.QuadPart);
			windata.gameinput.dt /= timeFrequency.QuadPart;

			windata.gameoutput.audio.framesToWrite = (oouint)(windata.gameinput.dt*audioFramesPerSecond);
			if(windata.gameoutput.audio.framesToWrite>audioBufferSize) {
				windata.gameoutput.audio.framesToWrite = audioBufferSize;
			}

			advanceGame(windata.gameMemory,&windata.gameinput,&windata.gameoutput);

			win32AudioOutput(windata.gameoutput.audio.buffer, windata.gameoutput.audio.framesToWrite);
			renderGraphics(&windata);
		}
		
		lastTime = time;
		
	}

	ReleaseDC(windata.window, windata.windowDC);

	destroyGraphics(&windata);
	win32AudioDestroy();
	free(windata.gameMemory);
	free(windata.gameoutput.audio.buffer);

	return (msg.wParam);
}


