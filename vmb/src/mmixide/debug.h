extern void new_memory_view(int i);
extern void new_register_view(int i);
extern void memory_update(void);

extern int set_breakpoint(int file_no, int line_no);
extern int del_breakpoint(int file_no, int line_no);
extern void clear_breakpoints(unsigned char file);
extern unsigned int show_special_registers;
extern INT_PTR CALLBACK OptionSpecialDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );
extern INT_PTR CALLBACK OptionDebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );
extern void set_debug_windows(void);
extern int break_at_Main;
extern int trace;
extern int show_os;
