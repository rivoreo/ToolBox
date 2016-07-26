/*	link - toolbox
	Copyright 2007-2015 PC GO Ld.

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>

#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(_WCE) && !defined(_WIN32_WNT_NATIVE)
#include <windows.h>
int __stdcall CreateHardLinkW(const wchar_t *, const wchar_t *, SECURITY_ATTRIBUTES *);
#else
#include <unistd.h>
#endif

void welcome(){
	printf("link - toolbox\nCopyright 2007-2015 PC GO Ld.\nCreate a file link to an existing file.\n\n");
}

int main(int argc, char *argv[]){
	if(argc > 3){
		fprintf(stderr,"too many arguments\n");
		return 1;
	}
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
	//wchar_t *file1;
	//wchar_t *file2;
	//size_t len1;
	//size_t len2;
#ifdef _WIN32_WCE
	welcome();
	fprintf(stderr, "Make link is not support on your system\n");
	return -1;
#else
	//fprintf(stderr, "Sorry, we made this without a way for the time being on Windows\n");
	if(argc < 3 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
		welcome();
		fprintf(stderr,"Usage: link.exe <target> <name>\n");
		return -1;
	}
	size_t len1 = strlen(argv[1]) + 1;
	size_t len2 = strlen(argv[2]) + 1;
	wchar_t file1[len1];
	wchar_t file2[len2];
	mbstowcs(file1, argv[1], len1);
	mbstowcs(file2, argv[2], len2);
	if(!CreateHardLinkW(file2, file1, NULL)) {
		fprintf(stderr, "link failed, %s\n", strerror(errno));
		return 2;
	}
	return 0;
#endif
#else
	if(argc < 3 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
		welcome();
		fprintf(stderr,"Usage: link <target> <name>\n");
		return -1;
	}
	int ret = link(argv[1], argv[2]);
	if(ret < 0) fprintf(stderr, "link failed, %s\n", strerror(errno));
	return ret;
#endif
}
