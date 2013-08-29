



extern int cur_file; /* index of the current file in |filename| */
extern int line_no; /* current position in the file */
extern bool line_listed; /* have we listed the buffer contents? */

extern char *filename[257];
extern int err_count; /* this many errors were found */
extern char *src_file_name; /* name of the \MMIXAL\ input file */
extern char obj_file_name[FILENAME_MAX+1]; /* name of the binary output file */
extern char listing_name[FILENAME_MAX+1]; /* name of the optional listing file */
extern FILE *src_file, *obj_file, *listing_file;
extern int expanding; /* are we expanding instructions when base address fail? */
extern int buf_size; /* maximum number of characters per line of input */
extern void flush_listing_line(char *s);
extern int mmixal(void);
extern char *special_name[32];
extern char mmix_special_registers[], mmix_predefined[], mmix_opcodes[];
extern int mmo_ptr;
extern void mmix_exit(int returncode);
#define MMIX_DISCONNECTED 0
#define MMIX_CONNECTED    1
#define MMIX_OFF          2
#define MMIX_ON           3
#define MMIX_STOPPED      4
#define MMIX_RUNNING      5
#define MMIX_HALTED       6


extern int mmix_status; 
extern void mmix_stopped(octa loc);
extern char full_mmo_name[];
extern void set_mmix_status(int status);