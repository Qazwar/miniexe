#include "getapi.h"
#include "api.h"


// converts string to lowercase
uint32_t crc32c(const char *s)
{
	int      i;
	uint32_t crc = 0;

	while (*s) {
		crc ^= (uint8_t)(*s++ | 0x20);

		for (i = 0; i<8; i++) {
			crc = (crc >> 1) ^ (0x82F63B71 * (crc & 1));
		}
	}
	return crc;
}

#ifndef ASM
LPVOID search_exp(LPVOID base, DWORD hash)
{
	PIMAGE_DOS_HEADER       dos;
	PIMAGE_NT_HEADERS       nt;
	DWORD                   cnt, rva, dll_h;
	PIMAGE_DATA_DIRECTORY   dir;
	PIMAGE_EXPORT_DIRECTORY exp;
	PDWORD                  adr;
	PDWORD                  sym;
	PWORD                   ord;
	PCHAR                   api, dll;
	LPVOID                  api_adr = NULL;

	dos = (PIMAGE_DOS_HEADER)base;
	nt = RVA2VA(PIMAGE_NT_HEADERS, base, dos->e_lfanew);
	dir = (PIMAGE_DATA_DIRECTORY)nt->OptionalHeader.DataDirectory;
	rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	// if no export table, return NULL
	if (rva == 0) return NULL;

	exp = (PIMAGE_EXPORT_DIRECTORY)RVA2VA(ULONG_PTR, base, rva);
	cnt = exp->NumberOfNames;

	// if no api names, return NULL
	if (cnt == 0) return NULL;

	adr = RVA2VA(PDWORD, base, exp->AddressOfFunctions);
	sym = RVA2VA(PDWORD, base, exp->AddressOfNames);
	ord = RVA2VA(PWORD, base, exp->AddressOfNameOrdinals);
	dll = RVA2VA(PCHAR, base, exp->Name);

	// calculate hash of DLL string
	dll_h = crc32c(dll);

	do {
		// calculate hash of api string
		api = RVA2VA(PCHAR, base, sym[cnt - 1]);
		// add to DLL hash and compare
		if (crc32c(api) + dll_h == hash) {
			// return address of function
			api_adr = RVA2VA(LPVOID, base, adr[ord[cnt - 1]]);
			return api_adr;
		}
	} while (--cnt && api_adr == 0);
	return api_adr;
}

LPVOID search_imp(LPVOID base, DWORD hash)
{
	DWORD                    dll_h, i, rva;
	PIMAGE_IMPORT_DESCRIPTOR imp;
	PIMAGE_THUNK_DATA        oft, ft;
	PIMAGE_IMPORT_BY_NAME    ibn;
	PIMAGE_DOS_HEADER        dos;
	PIMAGE_NT_HEADERS        nt;
	PIMAGE_DATA_DIRECTORY    dir;
	PCHAR                    dll;
	LPVOID                   api_adr = NULL;

	dos = (PIMAGE_DOS_HEADER)base;
	nt = RVA2VA(PIMAGE_NT_HEADERS, base, dos->e_lfanew);
	dir = (PIMAGE_DATA_DIRECTORY)nt->OptionalHeader.DataDirectory;
	rva = dir[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

	// if no import table, return
	if (rva == 0) return NULL;

	imp = (PIMAGE_IMPORT_DESCRIPTOR)RVA2VA(ULONG_PTR, base, rva);

	for (i = 0; api_adr == NULL; i++)
	{
		if (imp[i].Name == 0) return NULL;

		// get DLL string, calc crc32c hash
		dll = RVA2VA(PCHAR, base, imp[i].Name);
		dll_h = crc32c(dll);

		rva = imp[i].OriginalFirstThunk;
		oft = (PIMAGE_THUNK_DATA)RVA2VA(ULONG_PTR, base, rva);

		rva = imp[i].FirstThunk;
		ft = (PIMAGE_THUNK_DATA)RVA2VA(ULONG_PTR, base, rva);

		for (;; oft++, ft++)
		{
			if (oft->u1.Ordinal == 0) break;
			// skip import by ordinal
			if (IMAGE_SNAP_BY_ORDINAL(oft->u1.Ordinal)) continue;

			rva = oft->u1.AddressOfData;
			ibn = (PIMAGE_IMPORT_BY_NAME)RVA2VA(ULONG_PTR, base, rva);

			if ((crc32c((char*)ibn->Name) + dll_h) == hash) {
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
	PPEB                  peb;
	PPEB_LDR_DATA         ldr;
	PLDR_DATA_TABLE_ENTRY dte;
	LPVOID                api_adr = NULL;

#if defined(_WIN64)
	peb = (PPEB)__readgsqword(0x60);
#else
	peb = (PPEB)__readfsdword(0x30);
#endif

	ldr = (PPEB_LDR_DATA)peb->Ldr;

	// for each DLL loaded
	for (dte = (PLDR_DATA_TABLE_ENTRY)ldr->InLoadOrderModuleList.Flink;
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



int main(int argc, char *argv[])
{
	msvcrt_api->printf_("hello : %s",argv[0]);
	user32_api->MessageBoxA_(NULL,"It works!","Hi",MB_OK);
	return 0;
}