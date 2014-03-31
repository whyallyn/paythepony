#ifndef WINUTILS_H
#define WINUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <mmdeviceapi.h>
#include <initguid.h>
#include <endpointvolume.h>

#include "stringList.h"

// link in Window Multimedia API library
#pragma comment(lib, "Winmm")

// define GUIDs for audio interfaces used in setVolumeVista()
DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C,
            0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35,
            0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(IID_IAudioEndpointVolume, 0x5CDF2C82, 0x841E, 0x4546,
            0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A);

// Windows version definitions used by getWindowsOSVersion()
#define WINDOWS_8_1           631
#define WINDOWS_SERVER2012_R2 630 
#define WINDOWS_8             621
#define WINDOWS_SERVER2012    620
#define WINDOWS_7             611
#define WINDOWS_SERVER2008_R2 610
#define WINDOWS_VISTA         601
#define WINDOWS_SERVER2008    600
#define WINDOWS_SERVER2003_R2 521
#define WINDOWS_SERVER2003    520
#define WINDOWS_XP            510
#define WINDOWS_2000          500

// IsWow64Process is not available on all versions of Windows
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

WCHAR *genRandFilename_alloc(unsigned int filenameLen, LPCWSTR fileExtension);
DWORD setTiledWallpaper(LPCWSTR imagePath);
BOOL getPointerToResource(HMODULE hModule, int resource, WCHAR *resourceType, HGLOBAL *hResData, LPVOID *lpResource, DWORD *resourceSize);
BOOL writeResourceToFile(int resource, WCHAR *resourceType, WCHAR *filepath, DWORD dwFlagsAndAttributes);
DWORD getWindowsOSVersion();
BOOL isProcess64(HANDLE hProcess);
void setVolumeXP(DWORD volumeLevel);
void setVolumeVista(float volumeScalar);
BOOL recursiveDirList(TCHAR *fp, stringList *dirList);
DWORD getSizeOfPEfromSectionTable(HANDLE hFile);

#endif //WINUTILS_H