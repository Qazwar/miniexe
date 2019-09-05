#include "getapi.h"
#include "api.h"

// converts string to lowercase
uint32_t crc32c(const char* s)
{
	uint32_t crc = 0;

	while (*s)
	{
		crc ^= (uint8_t)(*s++ | 0x20);

		for (int i = 0; i < 8; i++)
		{
			crc = crc >> 1 ^ 0x82F63B71 * (crc & 1);
		}
	}
	return crc;
}

#ifndef ASM
LPVOID search_exp(LPVOID base, DWORD hash)
{
	LPVOID api_adr = NULL;

	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
	PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, base, dos->e_lfanew);
	PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY)nt->OptionalHeader.DataDirectory;
	DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	// if no export table, return NULL
	if (rva == 0) return NULL;

	PIMAGE_EXPORT_DIRECTORY exp = (PIMAGE_EXPORT_DIRECTORY)RVA2VA(ULONG_PTR, base, rva);
	DWORD cnt = exp->NumberOfNames;

	// if no api names, return NULL
	if (cnt == 0) return NULL;

	PDWORD adr = RVA2VA(PDWORD, base, exp->AddressOfFunctions);
	PDWORD sym = RVA2VA(PDWORD, base, exp->AddressOfNames);
	PWORD ord = RVA2VA(PWORD, base, exp->AddressOfNameOrdinals);
	PCHAR dll = RVA2VA(PCHAR, base, exp->Name);

	// calculate hash of DLL string
	const DWORD dll_h = crc32c(dll);

	do
	{
		// calculate hash of api string
		PCHAR api = RVA2VA(PCHAR, base, sym[cnt - 1]);
		// add to DLL hash and compare
		if (crc32c(api) + dll_h == hash)
		{
			// return address of function
			api_adr = RVA2VA(LPVOID, base, adr[ord[cnt - 1]]);
			return api_adr;
		}
	}
	while (--cnt && api_adr == 0);
	return api_adr;
}

LPVOID search_imp(LPVOID base, DWORD hash)
{
	LPVOID api_adr = NULL;

	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
	PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, base, dos->e_lfanew);
	PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY)nt->OptionalHeader.DataDirectory;
	DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

	// if no import table, return
	if (rva == 0) return NULL;

	PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)RVA2VA(ULONG_PTR, base, rva);

	for (DWORD i = 0; api_adr == NULL; i++)
	{
		if (imp[i].Name == 0) return NULL;

		// get DLL string, calc crc32c hash
		PCHAR dll = RVA2VA(PCHAR, base, imp[i].Name);
		DWORD dll_h = crc32c(dll);

		rva = imp[i].OriginalFirstThunk;
		PIMAGE_THUNK_DATA oft = (PIMAGE_THUNK_DATA)RVA2VA(ULONG_PTR, base, rva);

		rva = imp[i].FirstThunk;
		PIMAGE_THUNK_DATA ft = (PIMAGE_THUNK_DATA)RVA2VA(ULONG_PTR, base, rva);

		for (;; oft++, ft++)
		{
			if (oft->u1.Ordinal == 0) break;
			// skip import by ordinal
			if (IMAGE_SNAP_BY_ORDINAL(oft->u1.Ordinal)) continue;

			rva = (DWORD)oft->u1.AddressOfData;
			PIMAGE_IMPORT_BY_NAME ibn = (PIMAGE_IMPORT_BY_NAME)RVA2VA(ULONG_PTR, base, rva);

			if (crc32c((char*)ibn->Name) + dll_h == hash)
			{
				api_adr = (LPVOID)ft->u1.Function;
				break;
			}
		}
	}
	return api_adr;
}

/**F*********************************************
*
* Obtain address of API from PEB based on hash
*
************************************************/
LPVOID get_api(DWORD dwHash)
{
	PPEB peb;
	PPEB_LDR_DATA ldr;
	LPVOID api_adr = NULL;

#if defined(_WIN64)
	peb = (PPEB)__readgsqword(0x60);
#else
	peb = (PPEB)__readfsdword(0x30);
#endif

	ldr = (PPEB_LDR_DATA)peb->Ldr;

	// for each DLL loaded
	for (PLDR_DATA_TABLE_ENTRY dte = (PLDR_DATA_TABLE_ENTRY)ldr->InLoadOrderModuleList.Flink;
	     dte->DllBase != NULL && api_adr == NULL;
	     dte = (PLDR_DATA_TABLE_ENTRY)dte->InLoadOrderLinks.Flink)
	{
#ifdef IMPORT
		api_adr = search_imp(dte->DllBase, dwHash);
#else
		api_adr = search_exp(dte->DllBase, dwHash);
#endif
	}
	return api_adr;
}
#endif


int main(int argc, char* argv[])
{
	msvcrt_api->printf_("hello : %s", argv[0]);
	user32_api->MessageBoxA_(NULL, "It works!", "Hi",MB_OK);
	return 0;
}
