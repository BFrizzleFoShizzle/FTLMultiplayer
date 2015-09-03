#include "FTLMultiplayer.h"
#include "TextHelper.h"
#include "FTLShipSelector.h"
#include "FTLDraw.h"
#include "FTLAPI.h"
#include <cstdlib>
#include <windows.h>
#include <cstdio>
#include <gl\GL.h>

#define DEBUG true
//black magic for getting a handle on our own DLL
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

HANDLE FTLProcess;

void drawShit(void) {
	char output[10];
	if(playerShip != NULL) {
		//error checking is for bitches
		sprintf(output, "%i", playerShip->health);
		//sprintf(output, "%i02", playerShip->health);
		drawString(200.0f,200.0f,output);
	}
};

DWORD WINAPI FTLM_Main (LPVOID lpParam)
{
	FTLProcess = GetCurrentProcess();
	//this will also hook glfinish
	FTLSSMain();
	//add our draw hook
	addDrawHook(drawShit);
	//set up pointers to game stuff
	while(playerShip == NULL){
		ReadProcessMemory(FTLProcess,(VOID*)(0x400000+0x39BA90),&playerShip,4,NULL);
		Sleep(100);
	}
	return 0;
};

BOOL WINAPI DllMain (HINSTANCE hModule, DWORD dwAttached, LPVOID lpvReserved)
{
    if (dwAttached == DLL_PROCESS_ATTACH) {
        CreateThread(NULL,0,&FTLM_Main,NULL,0,NULL);
    }
    return 1;
};