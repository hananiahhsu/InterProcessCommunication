#include "pch.h"
#include <iostream>
#include <windows.h>

using namespace std;

#define COMMUNICATION_OBJECT_NAME L"__FILE_MAPPING__"
#define SYNCHRONIZING_MUTEX_NAME L"__TEST_MUTEX__"
#define WINDOW_CLASS_NAME L"__TMPWNDCLASS__"
#define BUTTON_CLOSE 100

typedef struct _tagCOMMUNICATIONOBJECT
{
	HWND hWndClient;
	BOOL bExitLoop;
	LONG lSleepTimeout;
}COMMUNICATIONOBJECT,*PCOMMUNICATIONOBJECT;


//========================================================
LRESULT CALLBACK WndProc(HWND hDlg,UINT uMsg, WPARAM wParam,LPARAM lParam);
HWND InitializeWnd();
PCOMMUNICATIONOBJECT pCommObject = NULL;
HANDLE hMapping = NULL;

//==========================================================




int main(int argc, char* argv[])
{
	cout << "Interprocess communication demo" << endl;
	HWND hWnd = InitializeWnd();
	if (!hWnd)
	{
		cout << "cannot create window!" << "error:\t" << GetLastError() << endl;
		return 1;
	}

	HANDLE hMutex = CreateMutex(NULL,FALSE,SYNCHRONIZING_MUTEX_NAME);
	if (!hMutex)
	{
		cout << "cannot create mutex!" << endl << "error:\t" << GetLastError() << endl;
		return 1;
	}

	hMapping = CreateFileMapping((HANDLE)-1,NULL,PAGE_READWRITE,0,sizeof(COMMUNICATIONOBJECT),COMMUNICATION_OBJECT_NAME);
	if (!hMapping)
	{
		cout << "cannot create mapping object" << endl << "error:\t"  << GetLastError() << endl;
	}

	pCommObject = (PCOMMUNICATIONOBJECT)MapViewOfFile(hMapping,FILE_MAP_WRITE,0,0,0);
	if (pCommObject)
	{
		pCommObject->bExitLoop = FALSE;
		pCommObject->hWndClient = hWnd;
		pCommObject->lSleepTimeout = 250;//
		UnmapViewOfFile(pCommObject);
	}


	//--Red clor   -- blue color
	STARTUPINFO startupInfoRed = {0};
	PROCESS_INFORMATION processInformationRed = {0};
	STARTUPINFO startupInfoBlue = {0};
	PROCESS_INFORMATION processInformationBlue = {0};

	BOOL bSuccess = CreateProcessW(L"J:\\ReadMem\\interprocesscommunication\\InterProcCommunication\\x64\\Debug\\InterProcTask.exe",(LPWSTR)L"red",NULL,NULL,FALSE,0,NULL,NULL,
		                          &startupInfoRed,&processInformationRed);

	if (!bSuccess)
	{
		cout << "cannot create process --red" << endl << "error:\t" << GetLastError() << endl;
		return 1;
	}

	bSuccess = CreateProcessW(L"J:\\ReadMem\\interprocesscommunication\\InterProcCommunication\\x64\\Debug\\InterProcTask.exe", (LPWSTR)L"blue", NULL, NULL, FALSE, 0, NULL, NULL,
		&startupInfoBlue, &processInformationBlue);

	if (!bSuccess)
	{
		cout << "cannot create process --blue" << endl << "error:\t" << GetLastError() << endl;
		return 1;
	}

	MSG msg = {0};

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//GetModuleHandle()
	UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(NULL));

	CloseHandle(hMapping);
	CloseHandle(hMutex);
	cout << "End program..." << endl;
	return 0;
}



///////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case BUTTON_CLOSE:
					PostMessage(hWnd,WM_CLOSE,0,0);
					break;
			}
			break;
		}
		case WM_DESTROY :
		{
			pCommObject = (PCOMMUNICATIONOBJECT)MapViewOfFile(hMapping,FILE_MAP_WRITE,0,0,0);
			if (pCommObject)
			{
				pCommObject->bExitLoop = TRUE;
				UnmapViewOfFile(pCommObject);
			}
			PostQuitMessage(0);
			break;
		}

		default:
		{
			return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}
	}


	return 0;
}


////
HWND InitializeWnd()
{
	WNDCLASSEX wndEx;
	wndEx.cbSize = sizeof(WNDCLASSEX);
	wndEx.style = CS_HREDRAW | CS_VREDRAW;
	wndEx.lpfnWndProc = WndProc;
	wndEx.cbClsExtra = 0;
	wndEx.cbWndExtra = 0;
	wndEx.hInstance = GetModuleHandle(NULL);//-----------
	wndEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndEx.lpszMenuName = NULL;
	wndEx.lpszClassName = WINDOW_CLASS_NAME;//
	wndEx.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndEx.hIcon = LoadIcon(wndEx.hInstance,MAKEINTRESOURCE(IDI_APPLICATION));
	wndEx.hIconSm = LoadIcon(wndEx.hInstance,MAKEINTRESOURCE(IDI_APPLICATION));
	
	if (!RegisterClassEx(&wndEx))
	{
		return NULL;
	}
	HWND hWnd = CreateWindow(wndEx.lpszClassName,L"Interprocess communication demo",
		WS_OVERLAPPEDWINDOW,200,200,400,300,NULL,NULL,wndEx.hInstance,NULL);
	if (!hWnd)
	{
		return NULL;
	}

	HWND hButton = CreateWindow(L"BUTTON", L"Close",
		WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP,
		275, 225, 100, 25, hWnd, (HMENU)BUTTON_CLOSE,wndEx.hInstance ,NULL);

	//The area being draw with red or blue
	HWND hStatic = CreateWindow(L"STATIC", L"xxx",
		WS_CHILD | WS_VISIBLE,
		10, 10, 365, 205, hWnd, NULL, wndEx.hInstance, NULL);

	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);
	return hStatic;
}