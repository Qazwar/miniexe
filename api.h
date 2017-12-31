#pragma once
#include <windows.h>


typedef int WINAPI MessageBoxA_(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
typedef LPVOID WINAPI VirtualAlloc_(
	_In_opt_ LPVOID lpAddress,
	_In_     SIZE_T dwSize,
	_In_     DWORD  flAllocationType,
	_In_     DWORD  flProtect
);
typedef LPTSTR WINAPI GetCommandLineA_(void);

typedef HMODULE WINAPI LoadLibraryA_(_In_ LPCSTR lpLibFileName);
typedef void exit_(int _Code);
typedef int  printf_(char const* const _Format, ...);


typedef struct _kernel32_api {
	LoadLibraryA_	*LoadLibraryA_;
	VirtualAlloc_   *VirtualAlloc_;
	GetCommandLineA_ *GetCommandLineA_;
}kernel32_api_t,*pkernel32_api_t;

typedef struct _user32_api {
	MessageBoxA_	*MessageBoxA_;
}user32_api_t,*puser32_api_t;

typedef struct _msvcrt_api {
	exit_		*exit_;
	printf_		*printf_;
}msvcrt_api_t,*pmsvcrt_api_t;


void get_kernel32_api();
void get_msvcrt_api();
void get_user32_api();
//int init_global_api();

pkernel32_api_t kernel32_api;
puser32_api_t   user32_api;
pmsvcrt_api_t   msvcrt_api;
