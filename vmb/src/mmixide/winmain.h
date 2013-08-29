extern HINSTANCE hInst;
extern HWND	  hMainWnd, hError;
extern HMENU	  hMenu;
extern HWND	  hSplitter;
extern HWND      hButtonBar;
extern char fullname[];
extern void ide_status(char *message);
extern void ide_add_error(char *message, int line_no);
extern void ide_clear_error(void);
extern void update_breakpoints(void);
extern int assemble_if_needed(void);
extern void new_errorlist(void);
extern void new_edit(void);
extern void ide_exit_ignore(int returncode);
extern void ed_open_file(char *name);
#define WM_MMIX_STOPPED (WM_USER+1)
