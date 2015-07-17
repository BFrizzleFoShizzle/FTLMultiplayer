#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <gl\GL.h>
#include "FTLMultiplayer.h"
#include "FTLShip.h"

//black magic for getting a handle on our own DLL
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

const float charW = 8.0f;

HANDLE FTLProcess;
DWORD glFinishPointer;
GLuint texFont;
GLubyte texFontData[128][128][4];

ship* ships = NULL;
ship* playerShip = NULL;

char ourDirectory[MAX_PATH] = {0};

void drawShit(void) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texFont);
	glBindTexture(GL_TEXTURE_2D, texFont);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, texFontData);
	char output[10];
	if(playerShip != NULL) {
		//error checking is for bitches
		sprintf(output, "%i", playerShip->health);
		//sprintf(output, "%i02", playerShip->health);
		drawString(200.0f,200.0f,output);
	}
}

void hookOpenGLFinish(void) {
	//EBP, EBX and EDI are pushed by VC++ automatically before this ASM block
	//code here is safe, just don't use any variables, cause they'll prolly screw up the registers
	//if you need variables, create a function and call it here, that will preserve the registers

	drawShit();

	//reset registers and jump to opengl code
	__asm
	{
		pop edi;
		pop ebx;
		pop ebp;
		jmp glFinishPointer;
	}
}

DWORD WINAPI FTLM_Main (LPVOID lpParam)
{
	// glFinish hook start: push *hookOpenGLFinish, return (jumps to first instruction in hookOpenGLFinish)
	BYTE glFinishHook[6] = {0x68, 0x00, 0x00, 0x00, 0x00, 0xC3};
	// write bytes in reverse orderWriteProcessMemory
	glFinishHook[1] = ((DWORD)hookOpenGLFinish>>0)&0xFF;
	glFinishHook[2] = ((DWORD)hookOpenGLFinish>>8)&0xFF;
	glFinishHook[3] = ((DWORD)hookOpenGLFinish>>16)&0xFF;
	glFinishHook[4] = ((DWORD)hookOpenGLFinish>>24)&0xFF;
	FTLProcess = GetCurrentProcess();
	// get pointer of actual glFinish function
	glFinishPointer = (DWORD)GetProcAddress(GetModuleHandle("OPENGL32.dll"), "glFinish");
	char message[50];
	// write hook to FTL process memory, cbf checking for errors
	SIZE_T bytesWritten = 0;
	if(WriteProcessMemory(FTLProcess, (VOID*)(0x0025DBB8+0x00400000), &glFinishHook, sizeof(glFinishHook), &bytesWritten))
	{
		sprintf(message, "OPENGL32.dll glFinish found at: %x, wrote %lu", glFinishPointer, bytesWritten, sizeof(glFinishHook));
		MessageBox(NULL, message, "Success!", MB_OK + MB_ICONINFORMATION);
	} else {
		sprintf(message, "Failiure: %i", GetLastError());
		MessageBox(NULL, message, "Fail!", MB_OK + MB_ICONINFORMATION);
	}
	// glFinish hook end

	//font loading stuff
	//get our DLL's path, in a useful form
	WCHAR wcDllPath[MAX_PATH] = {0};
	GetModuleFileNameW((HINSTANCE)&__ImageBase, wcDllPath, _countof(wcDllPath));
	wcstombs(ourDirectory,wcDllPath,MAX_PATH);
	//strip DLL name
	*(strrchr(ourDirectory, '\\')+1) = '\x00';
	//work out file path to bitmap font
	char bitmapFile[MAX_PATH];
	FILE* fp;
	sprintf(bitmapFile, "%sfont.bmp", ourDirectory);
	fp = fopen(bitmapFile, "r");
	if(!fp) {
		MessageBox(NULL, "Missing fonts file! D: should be...", "Fail!", MB_OK + MB_ICONINFORMATION);
		MessageBox(NULL, bitmapFile, "Fail!", MB_OK + MB_ICONINFORMATION);
	}
	//load the bitmap font from our dir
	readBitmapFont(fp);
	//set up pointers to game stuff
	while(playerShip == NULL){
		ReadProcessMemory(FTLProcess,(VOID*)(0x400000+0x39BA90),&playerShip,4,NULL);
		Sleep(100);
	}
	return 0;
}

void drawString(float x, float y, char* text){
	while(*text != 0) {
		drawChar(x, y, *text);
		x+=charW;
		text++;
	}
}


void drawChar(float x, float y, char text) {
	//width/height of a char in texture space
	float charSize = 1.0f/16;
	//t (y) texture coord of char
	float charT = ((float)(text%16))*charSize;
	//s (x) texture coord of char
	float charS = ((float)(text/16))*charSize;
	
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	 glTexCoord2f(charS, charT);
	 glVertex3f(x, y, 0.0); 
	 glTexCoord2f(charS+charSize, charT);
	 glVertex3f(x+charW, y, 0.0); 
	 glTexCoord2f(charS+charSize, charT+charSize);
	 glVertex3f(x+charW, y+charW, 0.0); 
	 glTexCoord2f(charS, charT+charSize);
	 glVertex3f(x, y+charW, 0.0); 
	glEnd();
}

void readBitmapFont(FILE* fp) {
	//read up to the image data offset
	for(int i=0;i<0xA;i++) {
		fgetc(fp);
	}
	int dataOffset = 0;
	fread(&dataOffset, 4, 1, fp);
	//discard everything but the raw image data
	for(int i=0;i<dataOffset-0xE;i++) {
		fgetc(fp);
	}
	//read in image
	//(128*128)/8 = amount of raw data
	for(int i=0;i<(128*128)/8;i++) {
		int currByte = fgetc(fp);
		for(int currBit=0;currBit<8;currBit++) {
			int x = ((i*8)+currBit)%128;
			//fix
			int y = 128-(((i*8)+currBit)/128);
			//I'm not american, shocking!
			int colour = ((currByte>>(7-currBit))&1)*255;
			texFontData[y][x][0] = colour;
			texFontData[y][x][1] = colour;
			texFontData[y][x][2] = colour;
			texFontData[y][x][3] = colour;
		}
	}
}

BOOL WINAPI DllMain (HINSTANCE hModule, DWORD dwAttached, LPVOID lpvReserved)
{
    if (dwAttached == DLL_PROCESS_ATTACH) {
        CreateThread(NULL,0,&FTLM_Main,NULL,0,NULL);
    }
    return 1;
}