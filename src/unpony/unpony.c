#include "unpony.h"

// global variables
char dialogString[DIALOG_STRLEN] = {0};
HWND g_progressBar = NULL;
TCHAR *efilename = NULL;
HANDLE efile = NULL;
HANDLE decryptThread = NULL;
TCHAR *dfilename = NULL;
HANDLE dfile = NULL;	

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	INITCOMMONCONTROLSEX InitCtrlEx;

	// insure that common control DLL (Comctl32.dll) is loaded
	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&InitCtrlEx);

	// open unpony window
	DialogBoxParam(hInstance, (LPCTSTR)IDD_DIALOG1, 0, mainDialog, 0);

	return 0;
}

// main dialog menu
BOOL CALLBACK mainDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {

		case WM_INITDIALOG:
			// load icon
			SetClassLongPtr(hDlg, GCLP_HICON, (long)LoadIcon(0, IDI_APPLICATION));
			return TRUE;

		case WM_COMMAND:
			switch(wParam) {

				case IDMYACTION:
					MessageBoxA(hDlg, "To unlock your files, please contact your friendly neighborhood Red Team.", "Contact", MB_OK);
					break;

				case IDOK:
					if (!GetDlgItemTextA(hDlg, IDMYPASS, dialogString, DIALOG_STRLEN)) {
						// close dialog if no password input
						EndDialog(hDlg, 0);
					}
					else {
						// decrypt file
						DialogBoxParam(0, (LPCTSTR)IDD_DIALOG2, hDlg, decryptDialog, (LPARAM)dialogString);
						EndDialog(hDlg, 0);
					}
					break;

				case IDCANCEL:
					EndDialog(hDlg, 0);
					break;
			}
		}
	
	return FALSE;
}

// decryption windows with progress bar
BOOL CALLBACK decryptDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	unsigned char *dialogString = (unsigned char *)&lParam;
	LARGE_INTEGER fs = {0};

	switch(uMsg) {

		case WM_INITDIALOG:

			// load icon
			SetClassLongPtr(hDlg, GCLP_HICON, (long)LoadIcon(0, IDI_APPLICATION));

			// create progress bar
			g_progressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 116-38, 15+28, 52+208, 8+29, hDlg, NULL, NULL, NULL);

			// get a handle to this file
			efilename = (TCHAR *)calloc(MAX_PATH + 1, sizeof(TCHAR));
			GetModuleFileName(NULL, efilename, MAX_PATH);
			efile = CreateFile(efilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (efile == INVALID_HANDLE_VALUE) {
				MessageBoxA(hDlg, "Can not open this file!!! Decryption failed!!!", "UNPONY", MB_OK);
				free(efilename);
				DestroyWindow(g_progressBar);
				EndDialog(hDlg, 0);
				return FALSE;
			}
			
			// calculate progress bar percent based on file size
			if (!GetFileSizeEx(efile, &fs)) {
				MessageBoxA(hDlg, "Can not calculate file size!!! Decryption failed!!!", "UNPONY", MB_OK);
				CloseHandle(efile);
				free(efilename);
				DestroyWindow(g_progressBar);
				EndDialog(hDlg, 0);
				return FALSE;
			}
			SendMessage(g_progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, fs.LowPart / BUFFER_SIZE));
			SendMessage(g_progressBar, PBM_SETSTEP, (WPARAM) 1, 0);

			// start decryption thread
			decryptThread = CreateThread(NULL, 0, decryptAppendedFile, hDlg, 0, NULL);

			return TRUE;

		case WM_COMMAND:
			switch(wParam) {

				case IDCANCEL:
					// terminate thread
					TerminateThread(decryptThread, 0);
					CloseHandle(decryptThread);
					CloseHandle(efile);
					CloseHandle(dfile);
					DeleteFile(dfilename);
					free(efilename);
					DestroyWindow(g_progressBar);
					EndDialog(hDlg, 0);
					break;
			}
	}	
	
	return FALSE;
}

//SendMessage(g_progressBar, PBM_STEPIT, 0, 0);
DWORD WINAPI decryptAppendedFile(LPVOID lpParam) {
	
	HWND hDlg = (HWND)lpParam;
	size_t efilelen = 0;
	size_t dfilelen = 0;
	DWORD unponySize = 0;
	const char *result = NULL;

	// forward handle to end of unpony.exe
	if ( (unponySize = getSizeOfPEfromSectionTable(efile)) == FALSE ) {
		CloseHandle(efile);
		MessageBoxA(hDlg, "Error 02!!! Decryption failed!!!", "UNPONY", MB_OK);
		EndDialog(hDlg, 0);
		return 0;
	}
	if (SetFilePointer(efile, unponySize, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		CloseHandle(efile);
		MessageBoxA(hDlg, "Error 03!!! Decryption failed!!!", "UNPONY", MB_OK);
		EndDialog(hDlg, 0);
		return 0;
	}

	// create file to decrypt data into
	efilelen = wcslen(efilename);
	dfilelen = efilelen - wcslen(encryptedFileExt);
	dfilename = (TCHAR *)calloc(MAX_PATH + 1, sizeof(TCHAR));
	memcpy(dfilename, efilename, dfilelen * sizeof(TCHAR));
	DeleteFile(dfilename);
	dfile = CreateFile(dfilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dfile == INVALID_HANDLE_VALUE) {
		CloseHandle(efile);
		free(dfilename);
		MessageBoxA(hDlg, "Error 04!!! Decryption failed!!!", "UNPONY", MB_OK);
		EndDialog(hDlg, 0);
		return 0;
	}

	// decrypt appended file into outfile
	result = decryptFile_blowfish(efile, dfile, (unsigned char*)dialogString, g_progressBar);

	// close file handles
	CloseHandle(efile);
	CloseHandle(dfile);

	// delete outfile if decryption failed
	if (result != NULL) {
		DeleteFile(dfilename);
		free(dfilename);
		MessageBoxA(hDlg, result, "UNPONY", MB_OK);
		EndDialog(hDlg, 0);
		return 0;
	}

	free(dfilename);

	MessageBoxA(hDlg, "Decryption complete! Look in the current directory for the outfile.\n\nI hope that password was correct  ;-)", "UNPONY", MB_OK);						

	EndDialog(hDlg, 0);
	return 0;
}