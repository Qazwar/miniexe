#include <Windows.h>
#include "api.h"


extern int main(int arge, char* argv[]);

typedef enum _heap_type
{
	HEAP_BLOCK_FREE = (int)0xABABABAB,
	HEAP_BLOCK_USED = (int)0xCDCDCDCD,
} heap_type;

typedef struct _heap_header
{
	heap_type type;
	unsigned size;
	struct _heap_header* next;
	struct _heap_header* prev;
} heap_header;

#define ADDR_ADD(a, o)  ((char *)(a) + (o))
#define HEADER_SIZE (sizeof(heap_header))

static heap_header* list_head = NULL;


int mini_crt_heap_init()
{
	const unsigned heap_size = 1024 * 1024 * 32; // 32 MB heap size

	void* base = kernel32_api->VirtualAlloc_(0,
	                                         heap_size,
	                                         MEM_COMMIT | MEM_RESERVE,
	                                         PAGE_READWRITE);
	if (base == NULL)
	{
		return 0;
	}

	heap_header* header = (heap_header *)base;

	header->size = heap_size;
	header->type = HEAP_BLOCK_FREE;
	header->next = NULL;
	header->prev = NULL;

	list_head = header;
	return 1;
}

int mini_crt_io_init()
{
	return 1;
}

static void crt_fatal_error(const char* msg)
{
	msvcrt_api->printf_("fatal error: %s", msg);
	msvcrt_api->exit_(1);
}

void mini_crt_entry(void)
{
	int flag = 0;
	int argc = 0;
	char* argv[16];

	kernel32_api_t k_api;
	user32_api_t u_api;
	msvcrt_api_t m_api;

	kernel32_api = &k_api;
	user32_api = &u_api;
	msvcrt_api = &m_api;

	get_kernel32_api();
	if (!mini_crt_heap_init())
	{
		get_msvcrt_api();
		crt_fatal_error("Heap Initialize Failed!\n");
	}

	get_msvcrt_api();

	char* cl = kernel32_api->GetCommandLineA_();
	argv[0] = cl;
	argc++;
	while (*cl)
	{
		if (*cl == '\"' && flag == 0)
		{
			flag = 1;
		}
		else if (*cl == ' ' && flag == 0)
		{
			if (*(cl + 1))
			{
				argv[argc] = cl + 1;
				argc++;
			}
			*cl = '\0';
		}

		cl++;
	}

	get_user32_api();
	const int ret = main(argc, argv);
	msvcrt_api->exit_(ret);
}
