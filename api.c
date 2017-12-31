#include "api.h"
#include "getapi.h"


void get_kernel32_api() {
	kernel32_api->LoadLibraryA_ = (LoadLibraryA_*)get_api(crc32c("kernel32.dll")+crc32c("LoadLibraryA"));
	kernel32_api->VirtualAlloc_ = (VirtualAlloc_*)get_api(crc32c("kernel32.dll") + crc32c("VirtualAlloc"));
	kernel32_api->GetCommandLineA_ = (GetCommandLineA_*)get_api(crc32c("kernel32.dll") + crc32c("GetCommandLineA"));
}

void get_user32_api() {
	kernel32_api->LoadLibraryA_("user32.dll");
	user32_api->MessageBoxA_ = (MessageBoxA_*)get_api(crc32c("user32.dll") + crc32c("MessageBoxA"));
}

void get_msvcrt_api() {
	kernel32_api->LoadLibraryA_("msvcrt.dll");
	msvcrt_api->exit_ = (exit_*)get_api(crc32c("msvcrt.dll") + crc32c("exit"));
	msvcrt_api->printf_ = (printf_*)get_api(crc32c("msvcrt.dll") + crc32c("printf"));
}

//int init_global_api() {
//	get_kernel32_api();
//	get_user32_api();
//	get_msvcrt_api();
//}