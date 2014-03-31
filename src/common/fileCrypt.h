#ifndef FILECRYPT_H
#define FILECRYPT_H

#include <stdlib.h>
#include <Windows.h>
#include <tomcrypt.h>
#include <CommCtrl.h>

#define BUFFER_SIZE 4096

const char *encryptFile_blowfish(HANDLE infile, HANDLE outfile, unsigned char *passkey);
const char *decryptFile_blowfish(HANDLE infile, HANDLE outfile, unsigned char *passkey, HWND progressBar);

#endif // FILECRYPT_H