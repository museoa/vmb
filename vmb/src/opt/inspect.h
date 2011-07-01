#include "option.h"

#ifdef WIN32
extern HWND hMemory; 

extern INT_PTR CALLBACK   
MemoryDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );

extern void adjust_memory_tab(void);
/* call this function to tell the memory inspector that an update is due */
extern void mem_update(int inspector, unsigned int offset, int size);

#else
/* make it a no-op */
#define mem_update(inspector, offset, size)

#endif



enum mem_fmt {hex_format=0, unsigned_format=1, signed_format=2, ascii_format=3, float_format=4, double_format=5,last_format=5 };

enum chunk_fmt {byte_chunk=0, wyde_chunk=1,tetra_chunk=2,octa_chunk=3, last_chunk=3 };
/* list registers strictly in order of increasing offsets */
struct register_def
{ char *name;
  int nr;
  int offset;
  int size;
  enum chunk_fmt chunk;
  enum mem_fmt  format;
};

struct inspector_def {
	char *name;
	unsigned int size;
    int (*get_mem)(unsigned int offset, int size, unsigned char *buf);
	uint64_t address;  /* used for memory only */
	int num_regs;      /* 0 if memory */
	struct register_def *regs; /* NULL if memory */
};

extern struct inspector_def inspector[];
extern int insp;