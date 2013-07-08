#include "option.h"

#ifdef WIN32
extern HWND hMemory; 

extern INT_PTR CALLBACK   
MemoryDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );

/* call this to switch to inspector i */
extern void adjust_memory_tab(int i);
/* call this function to tell the memory inspector that an update is due */
extern void mem_update(unsigned int offset, int size);
/* call this function to tell a specific memory inspector i that an update is due */
extern void mem_update_i(int i, unsigned int offset, int size);

#else
/* make it a no-op */
#define mem_update(inspector, offset, size)

#endif



enum mem_fmt {hex_format=0, unsigned_format=1, signed_format=2, ascii_format=3, float_format=4, double_format=5,last_format=5 };

enum chunk_fmt {byte_chunk=0, wyde_chunk=1,tetra_chunk=2,octa_chunk=3, last_chunk=3 };

extern char *format_names[];

extern char *chunk_names[];

extern int chunk_to_str(char *str, unsigned char *buf, enum mem_fmt fmt, 
						int chunk_size, int column_digits);

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
	unsigned char *(*load)(unsigned int offset,int size); /* function to simulate load */
    void (*store)(unsigned int offset,int size, unsigned char *payload); /* same for store */
	enum mem_fmt format;
	enum chunk_fmt chunk;
	int de_offset;
	uint64_t address;  /* used for memory only */
	int num_regs;      /* 0 if memory */
	struct register_def *regs; /* NULL if memory */
};

typedef struct inspector_def inspector_def;

extern struct inspector_def inspector[];
