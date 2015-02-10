#include <windows.h>
#include <nt.h>
#include <unistd.h>

long int NTAPI LdrDisableThreadCalloutsForDll(void *dll);

void exit(int status) {
	_exit(status);
}

int __stdcall DllMainCRTStartup(void *dll, unsigned long int reason, void *reserved) {
	PROCESS_BASIC_INFORMATION pbi;
	NtQueryInformationProcess((void *)-1, ProcessBasicInformation, &pbi, sizeof pbi, NULL);
	if(dll == pbi.PebBaseAddress) {
		write(STDOUT_FILENO, "Toolbox!", 8);
		_exit(0);
	}
	if(reason == DLL_PROCESS_ATTACH) {
		LdrDisableThreadCalloutsForDll(dll);
	}
	return 1;
}
