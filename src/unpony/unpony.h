#ifndef UNPONY_H
#define UNPONY_H

#include <stdlib.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "resource.h"
#include "..\common\winUtils.h"
#include "..\common\fileCrypt.h"

#pragma comment(lib, "Comctl32")

#define DIALOG_STRLEN 80

static const TCHAR *encryptedFileExt = L".exe";

BOOL CALLBACK mainDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK decryptDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI decryptAppendedFile(LPVOID lpParam);

#endif // UNPONY_H