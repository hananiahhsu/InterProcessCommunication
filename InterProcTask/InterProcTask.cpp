// InterProcTask.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <windows.h>


#define COMMUNICATION_OBJECT_NAME L"__FILE_MAPPING__"
#define SYNCHRONIZING_MUTEX_NAME  L"__TEST_MUTEX__"

typedef struct _tagCOMMUNICATIONOBJECT
{
	HWND hWndClient;
	BOOL bExitLoop;
	LONG lSleepTimeout;
}COMMUNICATIONOBJECT,*PCOMMUNICATIONOBJECT;


int main(int argc,char* argv[])
{
    //std::cout << "Hello World!\n"; 

	HBRUSH hBrush = NULL;

	if (strcmp("blue",argv[0]) == 0)
	{
		hBrush = CreateSolidBrush(RGB(0,0,255));
	}
	else
	{
		hBrush = CreateSolidBrush(RGB(255,0,0));
	}

	HWND hWnd = NULL;
	HDC hDC = NULL;
	RECT rectClient = {0};
	LONG lWaitTimeout = 0;
	HANDLE hMapping = NULL;
	PCOMMUNICATIONOBJECT pCommunicationObject = NULL;
	BOOL bContinueLoop = TRUE;

	//Mutex--MUTEX_NAME,OBJECT_NAME
	HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS,FALSE, SYNCHRONIZING_MUTEX_NAME);
	hMapping = OpenFileMapping(FILE_MAP_READ,FALSE,   COMMUNICATION_OBJECT_NAME);

	if (hMapping)
	{
		while (bContinueLoop)//2018.11.7
		{
			WaitForSingleObject(hMutex,INFINITE);
			pCommunicationObject = (PCOMMUNICATIONOBJECT)MapViewOfFile(hMapping,FILE_MAP_READ,0,0,sizeof(COMMUNICATIONOBJECT));
			if (pCommunicationObject)
			{
				bContinueLoop = !pCommunicationObject->bExitLoop;
				hWnd = pCommunicationObject->hWndClient;
				lWaitTimeout = pCommunicationObject->lSleepTimeout;
				UnmapViewOfFile(pCommunicationObject);
				hDC = GetDC(hWnd);
				if (GetClientRect(hWnd,&rectClient))
				{
					FillRect(hDC,&rectClient,hBrush);
				}
				ReleaseDC(hWnd,hDC);
				Sleep(lWaitTimeout);

			}

			ReleaseMutex(hMutex);
		}
	}


	CloseHandle(hMapping);
	CloseHandle(hMutex);
	DeleteObject(hBrush);
	return 0;
}



