extern HINSTANCE hInst;
extern HWND	  hMainWnd, hError;
extern HMENU	  hMenu;
extern HWND	  hSplitter;
extern HWND      hButtonBar;
extern HWND hFileList;
extern HWND hSymbolTable;
extern HWND	  hEdit;
extern LONG_PTR ed_send(unsigned int msg,ULONG_PTR wparam,LONG_PTR lparam);
extern void ide_status(char *message);
extern void ide_add_error(char *message, int file_no, int line_no);
extern void ide_clear_error_marker(void);
extern void ide_clear_error_list(void);
extern void update_breakpoints(void);
extern int assemble_if_needed(int file_no);
extern void new_errorlist(void);
extern void new_edit(void);
extern void ide_exit_ignore(int returncode);
extern void ed_open_file(char *name);
extern void clear_stop_marker(void);
extern void set_edit_file(int file_no);
extern ide_mark_breakpoint(int file_no, int line_no);
extern void show_stop_marker(int file_no, int line_no);
extern void set_lineno_width(void);
extern void set_profile_width(void);
extern void update_profile(void);
#define WM_MMIX_STOPPED (WM_USER+1)
#define WM_MMIX_RESET (WM_USER+2)
#define WM_MMIX_LOAD (WM_USER+3)

#define MMIX_LINE_MARGIN 0
#define MMIX_PROFILE_MARGIN 1
#define MMIX_BREAKX_MARKER 1
#define MMIX_BREAK_MARGIN 2
#define MMIX_TRACE_MARKER 2
#define MMIX_TRACE_MARGIN 3

/* packing file and line in an LPARAM */
#define item_data(file_no, line_no) ((LPARAM)(((line_no)<<8)|((file_no)&0xff)))
#define item_line_no(data) ((int)((data)>>8))
#define item_file_no(data) ((int)((data)&0xff))
