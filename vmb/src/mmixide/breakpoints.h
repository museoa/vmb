extern void create_breakpoints(void);
extern int set_file_breakpoint(int file_no, int line_no, int mask);
extern void del_file_breakpoint(int file_no, int line_no, int mask);
extern void remove_file_breakpoints(int file_no);
extern void update_breakpoints(void);
/* remove all breakpoints for this file */
extern HWND hBreakpoints;
// extern INT_PTR CALLBACK BreakpointsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );
extern int break_at_symbol(int file_no, char *symbol);
