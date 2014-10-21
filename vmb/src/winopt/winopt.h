#ifndef _WINOPT_H_
#define _WINOPT_H_

#include <windows.h>

/* Sybols needed by winopt */

extern HWND hMainWnd;
extern HINSTANCE hInst;
extern int major_version, minor_version;
extern char version[];
extern char title[];
extern char *program_name;
extern char *defined;
extern char *programhelpfile;

extern void win32_message(char *msg);
extern void win32_error(int line, char *message);
extern void win32_error2(int line, char *message, char *info);

extern void win32_fatal_error(int line, char *message);
extern int xpos, ypos; /* Window position */

extern void set_option(char **option, char *str);

/* Symbols provided by winopt */

typedef INT8 int8_t;
typedef INT16 int16_t;
typedef INT32 int32_t;
typedef INT64 int64_t;
typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;

/* in util.c */
extern void uint64tohex(uint64_t u, char *c);
extern uint64_t strtouint64(char *arg);

/* in winabout.c */
extern INT_PTR CALLBACK    
AboutDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );

/* in windialog.c */
extern void register_subwindow(HWND h);
extern void unregister_subwindow(HWND h);
extern BOOL do_subwindow_msg(MSG *msg);

 
/* from winxy.c */
extern void set_xypos(HWND hWnd);
extern void get_xypos(void);

/* from winpos.c */
extern void set_pos_key(HWND hWnd, char *name);
extern void get_pos_key(int *Xpos, int *Ypos, char *name);



/* table of key value pairs to store in the registry terminated by a NULL key */

typedef struct {
	char *key;
	void *value;
	int type; } regtable[];

/* these values are used for the type field in the regtable.
   non negative type values between 0 and 31 are used for flags */
#define TYPE_DWORD (-1)
#define TYPE_STRING (-2)
#define KEY_FLAGS ((char *)1)


extern regtable regtab;

/* from winreg.c */
extern void write_regtab(char *program);
extern void read_regtab(char * program);
extern void parse_commandline(int argc, char *argv[]);


#if 0
extern void win32_log(char *msg);

extern void mem_update(unsigned int offset, int size);
/* call this function to tell a specific memory inspector i that an update is due */
extern void mem_update_i(int i, unsigned int offset, int size);

extern HWND hDebug;
extern INT_PTR CALLBACK   
DebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );

extern INT_PTR CALLBACK   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam ); /*needed */


extern LRESULT CALLBACK 
OptWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); /* provided */

              
extern int set_reg_DWORD(char *program, char *name, DWORD value);
extern DWORD get_reg_DWORD(char *program, char *name);



#endif

#endif