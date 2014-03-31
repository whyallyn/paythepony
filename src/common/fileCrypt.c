#include "fileCrypt.h"

/* register algorithms */
BOOL register_algs(void) {

	if (register_cipher (&blowfish_desc) == -1)
		return FALSE;

	if (register_hash(&sha256_desc) == -1)
		return FALSE;
	
	if (register_prng(&yarrow_desc) == -1)
		return FALSE;
	
	if (register_prng(&sprng_desc) == -1)
		return FALSE;

	return TRUE;
}

/* encrypt/decrypt file */
const char *fileCrypt(HANDLE infile, HANDLE outfile, const char *cipher, unsigned char *passkey, HWND progressBar, BOOL decrypt) {

	unsigned char plaintext[BUFFER_SIZE], ciphertext[BUFFER_SIZE] = {0};
	unsigned char key[MAXBLOCKSIZE], IV[MAXBLOCKSIZE] = {0};
	unsigned char inbuf[512] = {0}; // i/o block size
	unsigned long outlen, ivsize, x = 0;
	symmetric_CTR ctr = {0};
	int cipher_idx, hash_idx, ks, error = 0;
	prng_state prng = {0};
	DWORD  dwBytesRead, dwBytesWritten = 0;

	// register algorithms
	if (!register_algs())
		return "Failed to register algorithms";

	// find cipher
	cipher_idx = find_cipher(cipher);
	if (cipher_idx == -1)
		return "Failed to find cipher";

	// find SHA256 hash
	hash_idx = find_hash("sha256");
	if (hash_idx == -1)
		return "Failed to find SHA256";

	// setup sizes
	ivsize = cipher_descriptor[cipher_idx].block_length;
	ks = hash_descriptor[hash_idx].hashsize;
	if (cipher_descriptor[cipher_idx].keysize(&ks) != CRYPT_OK)
		return "Failed to setup key sizes";

	// hash passkey
	outlen = sizeof(key);
	if ( (error = hash_memory(hash_idx, passkey, strlen((char *)passkey), key, &outlen)) != CRYPT_OK )
		return "Failed to hash passkey";
	 
	// decrypt
	if (decrypt) {

		// read in IV
		if(!ReadFile(infile, IV, ivsize, &dwBytesRead, NULL) || dwBytesRead != ivsize)
			return "Error reading from infile";

		// initialize CTR mode with IV
		if ( (error = ctr_start(cipher_idx, IV, key, ks, 0, CTR_COUNTER_LITTLE_ENDIAN, &ctr)) != CRYPT_OK )
			return error_to_string(error);

		// read file and decrypt
		do {
			if (!ReadFile(infile, inbuf, sizeof(inbuf), &dwBytesRead, NULL))
				return "Error reading from infile";

			if ( (error = ctr_decrypt(inbuf, plaintext, dwBytesRead, &ctr)) != CRYPT_OK )
					return error_to_string(error);

			// update progress bar
			SendMessage(progressBar, PBM_STEPIT, 0, 0);
			
			if(!WriteFile(outfile, plaintext, dwBytesRead, &dwBytesWritten, NULL) || dwBytesWritten != dwBytesRead)
				return "Error writing to outfile";
		} while (dwBytesRead == sizeof(inbuf));
	} 
	
	// encrypt
	else {
		
		// Setup yarrow for random bytes for IV			
		if ( (error = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK )
			return  error_to_string(error);

		// read from random bytes for IV
		x = yarrow_read(IV, ivsize, &prng);
		if (x != ivsize)
			return "Error reading PRNG for IV required";
	 
		// write IV to output file
		if (!WriteFile(outfile, IV, ivsize, &dwBytesWritten, NULL) || dwBytesWritten != ivsize)
			return "Error writing IV to outfile";

		// initialize CTR mode with IV
		if ( (error = ctr_start(cipher_idx, IV, key, ks, 0, CTR_COUNTER_LITTLE_ENDIAN, &ctr)) != CRYPT_OK )
			return error_to_string(error);

		// read file and encrypt
		do {
			if (!ReadFile(infile, inbuf, sizeof(inbuf), &dwBytesRead, NULL))
				return "Error reading from infile";

			if ( (error = ctr_encrypt(inbuf, ciphertext, dwBytesRead, &ctr)) != CRYPT_OK )
				return error_to_string(error);

			if(!WriteFile(outfile, ciphertext, dwBytesRead, &dwBytesWritten, NULL) || dwBytesWritten != dwBytesRead)
				return "Error writing to outfile";
		} while (dwBytesRead == sizeof(inbuf));   
	}

	return NULL;
}

const char *encryptFile(HANDLE infile, HANDLE outfile, const char *cipher, unsigned char *passkey) {

	return fileCrypt(infile, outfile, cipher, passkey, NULL, FALSE);
}

const char *decryptFile(HANDLE infile, HANDLE outfile, const char *cipher, unsigned char *passkey, HWND progressBar) {

	return fileCrypt(infile, outfile, cipher, passkey, progressBar, TRUE);
}

const char *encryptFile_blowfish(HANDLE infile, HANDLE outfile, unsigned char *passkey) {

	return encryptFile(infile, outfile, "blowfish", passkey);
}

const char *decryptFile_blowfish(HANDLE infile, HANDLE outfile, unsigned char *passkey, HWND progressBar) {

	return decryptFile(infile, outfile, "blowfish", passkey, progressBar);
}