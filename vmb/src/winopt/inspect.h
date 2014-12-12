//include "option.h"
#ifndef INSPECT_H
#define INSPECT_H
#include "float.h"

extern int mem_min_width,mem_min_height;
extern void set_mem_font_metrics(void);
enum mem_fmt {hex_format=0, ascii_format=1, unsigned_format=2, signed_format=3,  float_format=4, last_format=4, user_format=5 };

enum chunk_fmt {byte_chunk=0, wyde_chunk=1,tetra_chunk=2,octa_chunk=3, last_chunk=3, user_chunk=4 };

extern char *format_names[];

extern char *chunk_names[];

extern int chunk_to_str(char *str, unsigned char *buf, enum mem_fmt fmt, 
						int chunk_size, int column_digits);


/* list registers strictly in order of increasing offsets */
#define REG_OPT_DISABLED 0x1
#define REG_OPT_SEPARATOR 0x2

struct register_def
{ char *name;	/* must not be NULL */
  int offset;   /* must not be negativ */
  int size;		
  enum chunk_fmt chunk;
  enum mem_fmt  format;
  unsigned char options;
};
typedef struct register_def register_def;

struct inspector_def {
	char *name;
	unsigned int size;
    int (*get_mem)(unsigned int offset, int size, unsigned char *buf);
	unsigned char *(*load)(unsigned int offset,int size); /* function to simulate load */
    void (*store)(unsigned int offset,int size, unsigned char *payload); /* same for store */
	enum mem_fmt format;
	enum chunk_fmt chunk;
	int de_offset; /* offset for the data editor, -1 if none*/
	int sb_rng; /* number of lines covered by scrollbar */
	int num_regs;      /* number of registers if register_def!=NULL */
	struct register_def *regs; /* NULL if memory */
	/* the rest can be initialized with zero */
	int address_width;   /*size of addess or register names column*/
	uint64_t address;  /* used for memory only */
	RECT edit_rect;
	HWND hWnd; /* the window where the edit rectangle is displayed */
    unsigned int sb_base; /* offset at base of scrollbar */
    int sb_cur; /* line corresponding to start of page */
    int lines; /* lines per page */
	unsigned int line_range; /* number byte per line */
	int width;  /* width in pixel */
	int height; /* height in pixel */
    int columns; 
	int column_width; /* column width in pixel */
	int column_digits; /* number of output characters per column */
    unsigned int mem_base;
	unsigned int mem_size; /* page currently displayed  from offset mem_base to mem_base+memsize*/
    unsigned char *mem_buf; /* memory buffer */
	unsigned int old_base; /* same for the proviously displayed memory to indicate changes */
    unsigned int old_size;
    unsigned char* old_mem;
	int change_address;
};

typedef struct inspector_def inspector_def;

extern struct inspector_def inspector[];
#ifdef WIN32

extern void update_max_regnames(inspector_def *insp);
/* call this if you change the register names dynamically */

extern HWND CreateMemoryDialog(HINSTANCE hInst,HWND hParent);

/* call this to switch to inspector i */
extern void SetInspector(HWND hWnd, inspector_def * insp);
/* call this function to tell the memory inspector that an update is due */
void MemoryDialogUpdate(HWND hMemory,inspector_def *insp, unsigned int offset, int size);


extern void adjust_mem_display(inspector_def *insp);
#else
/* make it a no-op */
#define mem_update(offset, size)

#endif


#endif