extern void new_memory_view(int i);
extern void new_register_view(int i);
extern void memory_update(void);
extern void add_line_loc(unsigned char file, int line_no, octa loc);
extern int set_breakpoint(unsigned char file, int line_no);
extern int del_breakpoint(unsigned char file, int line_no);
extern void clear_breakpoints(unsigned char file);