Warning
=======
Do not run paythepony.exe on a system with content you do not wish to lose or have damaged. You have been warned.


About
=====

Pay the Pony is hilarityware that uses the Reflective DLL injection library to inject into a remote process, encrypt and demand a ransom for files, and inflict My Little Pony madness on a system.

Pay the Pony has been tested and works on all Service Pack levels of Windows XP, 2003, 2003R2, Vista, 2008, 7, and 2008R2.

Overview
========

Pay the Pony marries the idea of ransomware and prankware and creates a new class of software: hilarityware. Pay the Pony was created to inflict hilarity on the students of network security competitions such as the Collegiate Cyber Defense Competitions (CCDC) and the Palmetto Cyber Defense Competition (PCDC). Pay the Pony is comprised of an injector, paythepony.exe, and a DLL, ponyware(32/64).dll.

The injector (paythepony.exe) performs the following:
* Sets the Windows Desktop wallpaper to ponies
* Changes the Windows cursor to ponies
* Injects the ponyware Dll into the specified process using the correct 32 or 64-bit version depending on the target process

The ponyware DLL (ponyware32.dll/ponyware64.dll) performs the following once injected into the target process:
* Starts a thread that repeatedly turns the Windows volume to maximum
* Repeatedly plays a 16-bit rendition of the My Little Pony theme song
* Encrypts all files in the "Documents"/"My Documents", "Desktop", and "Downloads" directories with Blowfish in CBC mode
* Prepends each file with a Portable Executable (PE) file loader that queries the user for a password to decrypt the encrypted file

Build
=====

Open 'solution/paythepony.sln' in Visual Studio C++ and build the solution in Release mode for Win32 to make paythepony.exe and ponyware32.dll. To be able to inject into 64-bit processes, the ponywareDll project should also be built in Release mode for x64 to produce ponyware64.dll.

Usage
=====

The ponyware32.dll and/or ponyware64.dll must be in the same directory as paythepony.exe in order for the ponyware DLL to be injected and run. The syntax to use paythepony.exe to run its features and inject the ponyware DLL into a host process via a process ID is the following:

> paythepony.exe pid

Pay the Pony should be run as the same user in the same session that Explorer.exe is running in order to ensure maximum hilarity and that the correct directories are chosen for encryption.
	
License
=======

Licensed under a 3 clause BSD license, please see LICENSE for details.

The My Little Pony cursors were acquired from https://sites.google.com/site/desertarcanacursors/my-little-pony and the 16-bit My Little Pony song from http://infinitydash.bandcamp.com/track/mlp-fim-intro-16-bit-remix. All artwork and music belong to their respective owners.

The My Little Pony franchise is the property of Hasbro Inc.