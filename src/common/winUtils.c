#include "winUtils.h"

static const WCHAR alphanum[] =
	L"0123456789"
	L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	L"abcdefghijklmnopqrstuvwxyz";

/* generate random filename of a specified length with a chosen filename extension */
WCHAR *genRandFilename_alloc(unsigned int filenameLen, LPCWSTR fileExtension) {

	unsigned int i = 0;
	WCHAR *filename = (WCHAR *)calloc(filenameLen + wcslen(fileExtension) + 1, sizeof(WCHAR));

	// seed rand() with time
	srand( (unsigned)time( NULL ) );

	// generate random filename
	for (i = 0; i < filenameLen; ++i) {
		filename[i] = alphanum[rand() % ((sizeof(alphanum) / sizeof(WCHAR)) - 1)];
	}

	// add extension to end of filename
	wcscpy_s(filename+i, wcslen(fileExtension) + 1, fileExtension);

	return filename;
}

/* set tiled wallpaper to imagePath.jpg
   if return value is not ERROR_SUCCESS, return value is a Windows error code */
DWORD setTiledWallpaper(LPCWSTR imagePath) {

	HKEY hkey = NULL;
	DWORD error = ERROR_SUCCESS;
	
	// open registry key HKCU\Control Panel\Desktop
	error = RegOpenKeyEx(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hkey);
	if (ERROR_SUCCESS != error) {
		return error;
	}

	// tile wallpaper
	error = RegSetValueEx(hkey, L"WallpaperStyle", 0, REG_SZ, (LPBYTE)L"0", sizeof(WCHAR));
	if (ERROR_SUCCESS != error) {
		RegCloseKey(hkey);
		return error;
	}
	error = RegSetValueEx(hkey, L"TileWallpaper", 0, REG_SZ, (LPBYTE)L"1", sizeof(WCHAR));
	if (ERROR_SUCCESS != error) {
		RegCloseKey(hkey);
		return error;
	}

	// close registry key
	RegCloseKey(hkey);

	// set wallpaper
	if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)imagePath, SPIF_UPDATEINIFILE)) {
		error = GetLastError();
		// this function often fails but doesn't return an error code
		if (ERROR_SUCCESS == error)
			error = ERROR_INVALID_FUNCTION;
		return error;
	} 

	return ERROR_SUCCESS;
}

/* get pointer to resource
   as well as the size of the resource
	 if no error, free hResData resource
	 if return value is FALSE, use GetLastError() to get the Windows error code */
BOOL getPointerToResource(HMODULE hModule, int resource, WCHAR *resourceType, HGLOBAL *hResData, LPVOID *lpResource, DWORD *resourceSize) {

	HRSRC   hResource = NULL;

	// determine location of normal pony cursor resource
	hResource = FindResource(hModule, MAKEINTRESOURCE(resource), resourceType);
	if (!hResource) {
		return FALSE;
	}

	// load resource into memory
	*hResData = LoadResource(hModule, hResource);
	if (!*hResData) {
		return FALSE;
	}

	// retrieve pointer to resource
	*lpResource = LockResource(*hResData);
	if (!*lpResource) {
		FreeResource(*hResData);
		return FALSE;
	}

	// get resource size
	*resourceSize = SizeofResource(hModule, hResource);
	if (!resourceSize) {
		FreeResource(*hResData);
		return FALSE;
	}

	return TRUE;
}

/* write resource to file
   if return value is FALSE, use GetLastError() to get the Windows error code */
BOOL writeResourceToFile(int resource, WCHAR *resourceType, WCHAR *filepath, DWORD dwFlagsAndAttributes) {
	
	HGLOBAL hResData        = NULL;
	LPVOID  lpResource      = NULL;
	DWORD   resourceSize    = 0;
	HANDLE  hFile           = NULL;
	DWORD   numBytesWritten = 0;

	// get pointer to resource and size
	if (!getPointerToResource(NULL, resource, resourceType, &hResData, &lpResource, &resourceSize)) {
		return FALSE;
	}

	// create file
	hFile = CreateFile(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, dwFlagsAndAttributes, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		FreeResource(hResData);
		return FALSE;
	}

	// write resource data to file
	if (!WriteFile(hFile, lpResource, resourceSize, &numBytesWritten, NULL) || numBytesWritten != resourceSize) {
		FreeResource(hResData);
		CloseHandle(hFile);
		return FALSE;
	}

	// close file and free resource
	CloseHandle(hFile);
	FreeResource(hResData);

	return TRUE;
}

/* returns Windows OS version based on the defines in winUtils.h */
DWORD getWindowsOSVersion() {
	
	OSVERSIONINFOEX vi = {0};
	
	// indicate that the extended data structure is being used
	vi.dwOSVersionInfoSize = sizeof(vi);

	// get Windows version information
	if (!GetVersionEx((LPOSVERSIONINFO)&vi)) {
		return FALSE;
	}

	// return OS version by define
	if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 3 && vi.wProductType == VER_NT_WORKSTATION) {
		return WINDOWS_8_1;
	}
	else if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 3 && vi.wProductType != VER_NT_WORKSTATION) {
		return WINDOWS_SERVER2012_R2;
	}
	else if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 2 && vi.wProductType == VER_NT_WORKSTATION) {
		return WINDOWS_8;
	}
	else if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 2 && vi.wProductType != VER_NT_WORKSTATION) {
		return WINDOWS_SERVER2012;
	}
	else if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 1 && vi.wProductType == VER_NT_WORKSTATION) {
		return WINDOWS_7;
	}
	else if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 1 && vi.wProductType != VER_NT_WORKSTATION) {
		return WINDOWS_SERVER2008_R2;
	}
	else if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 0 && vi.wProductType == VER_NT_WORKSTATION) {
		return WINDOWS_VISTA;
	}
	else if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 0 && vi.wProductType != VER_NT_WORKSTATION) {
		return WINDOWS_SERVER2008;
	}
	else if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2) != 0) {
		return WINDOWS_SERVER2003_R2;
	}
	else if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2) == 0) {
		return WINDOWS_SERVER2003;
	}
	else if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 1) {
		return WINDOWS_XP;
	}
	else if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 0) {
		return WINDOWS_2000;
	}
	else {
		return FALSE;
	}
}

/* checks if process is 64-bit or not */
BOOL isProcess64(HANDLE hProcess) {

	BOOL bIsProc64    = FALSE;
	BOOL bIsThisWow64 = FALSE;
	BOOL bIsProcWow64 = FALSE;
	LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
	
	// get address of IsWow64Process
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	// Windows isn't 64-bit if IsWow64Process() doesn't exist
	if (fnIsWow64Process != NULL) {
		// check if current process (32-bit) is running in WoW64
		// not handling if function fails -> guess not 64-bit
		if (fnIsWow64Process(GetCurrentProcess(), &bIsThisWow64)) {
			// Windows is 64-bit -> check if process is 64-bit
			if (bIsThisWow64) {
				fnIsWow64Process(hProcess, &bIsProcWow64);
				// process running in WoW64 means it's 32-bit
				bIsProc64 = !bIsProcWow64;
			}
		}
	}

	return bIsProc64;
}

/* sets volume for Windows XP type volume systems
   the volumeLevel is a number between 0 and 65535 */
void setVolumeXP(DWORD volumeLevel) {

	HMIXER hMixer = NULL;
	MIXERLINE ml = {0};
	MIXERLINECONTROLS mlc = {0};
	MIXERCONTROL mc = {0};
	MIXERCONTROLDETAILS mcd = {0};
	MIXERCONTROLDETAILS_UNSIGNED mcdu = {0};

	// obtain a handle to the mixer device
	mixerOpen(&hMixer, MIXER_OBJECTF_MIXER, 0, 0, 0);

	// get the speaker line of the mixer device	
	ml.cbStruct = sizeof(MIXERLINE);
	ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	mixerGetLineInfo((HMIXEROBJ) hMixer, &ml, MIXER_GETLINEINFOF_COMPONENTTYPE);

	// get the volume control of the speaker line
	mlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mlc.dwLineID = ml.dwLineID;
	mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mlc.cControls = 1;
	mlc.pamxctrl = &mc;
	mlc.cbmxctrl = sizeof(MIXERCONTROL);
	mixerGetLineControls((HMIXEROBJ) hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

	// set the volume level
	mcdu.dwValue = volumeLevel;
	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.hwndOwner = 0;
	mcd.dwControlID = mc.dwControlID;
	mcd.paDetails = &mcdu;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.cChannels = 1;
	mixerSetControlDetails((HMIXEROBJ) hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
}

/* sets volume for Vista and newer
   the volumeScalar is a float between 0.0 and 1.0 */
void setVolumeVista(float volumeScalar) {

	HRESULT hr;
	IMMDeviceEnumerator *deviceEnumerator = NULL;
	IMMDevice *defaultDevice = NULL;
	IAudioEndpointVolume *endpointVolume = NULL;
	
	// Initialize the COM library on the current thread
	CoInitialize(NULL);
	
	// create instance of MMDeviceEnumerator
	hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &IID_IMMDeviceEnumerator, (LPVOID *)&deviceEnumerator);

	// get default audio device
	hr = deviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(deviceEnumerator, eRender, eConsole, &defaultDevice);

	// release enumerator
	deviceEnumerator->lpVtbl->Release(deviceEnumerator);

	// get volume controls of default audio device
	hr = defaultDevice->lpVtbl->Activate(defaultDevice, &IID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);

	// release default audio device
	defaultDevice->lpVtbl->Release(defaultDevice);

	// set master volume of default audio device
	hr = endpointVolume->lpVtbl->SetMasterVolumeLevelScalar(endpointVolume, volumeScalar, NULL);

	// release volume controls of default audio device
	endpointVolume->lpVtbl->Release(endpointVolume);

	//Uninitialize the COM library
	CoUninitialize();
}

/* recursively parse directory for files and add to stringList
   requires a null terminated full directory path and an initialized stringList
   if return value is FALSE, use GetLastError() to get the Windows error code */
BOOL recursiveDirList(TCHAR *fp, stringList *dirList) {

	TCHAR path[MAX_PATH + 1] = {0};
	WIN32_FIND_DATA ffd = {0};
	HANDLE hFind = INVALID_HANDLE_VALUE;

	// add '\*' to directory path
	wcscat_s(path, sizeof(path) / sizeof(TCHAR), fp);
	wcscat_s(path, sizeof(path) / sizeof(TCHAR), L"\\*");

	// Find the first file in the directory.
	hFind = FindFirstFile(path, &ffd);
	if (INVALID_HANDLE_VALUE == hFind) {
		return FALSE;
	} 
   
	// recursively add all the files in the directory to stringList
	do {
		// combine filepath with filename
		memset(path, 0, sizeof(path));
		wcscat_s(path, sizeof(path) / sizeof(TCHAR), fp);
		wcscat_s(path, sizeof(path) / sizeof(TCHAR), L"\\");
		wcscat_s(path, sizeof(path) / sizeof(TCHAR), ffd.cFileName);

		// ignore "."
		if (wcslen(ffd.cFileName) == 1 && ffd.cFileName[0] == '.')
			continue;
		// ignore ".."
		else if (wcslen(ffd.cFileName) == 2 && ffd.cFileName[0] == '.' && ffd.cFileName[1] == '.')
			continue;
		// follow down directory tree
		else if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (!recursiveDirList(path, dirList))
				return FALSE;
		}
		// add file to stringList
		else
			dirList->append(dirList, path);
	} while (FindNextFile(hFind, &ffd) != 0);

	if (GetLastError() != ERROR_NO_MORE_FILES) {
		FindClose(hFind);
		return FALSE;
	}

	FindClose(hFind);
	return TRUE;
}

/* get size of PE by adding PointerToRawData and SizeOfRawData
   from the section table with the maximum PointerToRawData value
   if return value is FALSE, use GetLastError() to get the Windows error code */
DWORD getSizeOfPEfromSectionTable(HANDLE hFile) {
	
	unsigned char buffer[4096] = {0};
	DWORD dwBytesRead = 0;
	IMAGE_DOS_HEADER *dosheader = NULL;
	IMAGE_NT_HEADERS *ntheader = NULL;
	IMAGE_SECTION_HEADER *sectiontable = NULL;
	WORD i = 0;
	DWORD maxpointer = 0;
	DWORD pesize = 0;

	// read beginning of file into buffer
	if (!ReadFile(hFile, buffer, sizeof(buffer), &dwBytesRead, NULL))
		return FALSE;

	// locate and check that we have a PE header
	dosheader = (IMAGE_DOS_HEADER *)buffer;
	ntheader = (IMAGE_NT_HEADERS *)(buffer + dosheader->e_lfanew);
	if (dosheader->e_magic != IMAGE_DOS_SIGNATURE || ntheader->Signature != IMAGE_NT_SIGNATURE)
		return FALSE;

	// for the section table with the maximum PointerToRawData value
	// PE size = sectiontable.PointerToRawData + sectiontable.SizeOfRawData
	sectiontable = (IMAGE_SECTION_HEADER *)((unsigned char *)ntheader + sizeof(IMAGE_NT_HEADERS));
	for (i = 0; i < ntheader->FileHeader.NumberOfSections; i++) {
		if (sectiontable->PointerToRawData > maxpointer) {
			maxpointer = sectiontable->PointerToRawData;
			pesize = sectiontable->PointerToRawData + sectiontable->SizeOfRawData;
		}
		sectiontable++;
	}

	return pesize;
}