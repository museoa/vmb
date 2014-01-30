extern void new_memory_view(int i);
extern void new_register_view(int i);
extern void memory_update(void);
extern void debug_init(void);

extern int set_breakpoint(int file_no, int line_no);
extern int del_breakpoint(int file_no, int line_no);
extern void clear_breakpoints(unsigned char file);
extern unsigned int show_special_registers;
extern INT_PTR CALLBACK OptionSpecialDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );
extern INT_PTR CALLBACK OptionDebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );
extern void set_debug_windows(void);
extern int break_at_Main;
extern int break_after;

#define REG_LOCAL 0
#define REG_GLOBAL 1
#define REG_SPECIAL 2
#define REG_STACK 3


extern int show_debug_local;
extern int show_debug_global;
extern int show_debug_special;
extern int show_debug_regstack;
extern int show_debug_text;
extern int show_debug_data;
extern int show_debug_pool;
extern int show_debug_stack;
extern int show_debug_neg;
extern int show_trace;
extern int auto_connect;