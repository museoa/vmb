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

#define WIN_REGSTACK               (1<<REG_STACK)
#define WIN_LOCAL                  (1<<REG_LOCAL)
#define WIN_GLOBAL                 (1<<REG_GLOBAL)
#define WIN_SPECIAL                (1<<REG_SPECIAL)
#define WIN_TEXT                   (1<<4)
#define WIN_DATA                   (1<<5)
#define WIN_POOL                   (1<<6)
#define WIN_STACK                  (1<<7)
#define WIN_NEG					   (1<<8)

extern unsigned int show_debug_windows;