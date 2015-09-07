#ifndef _WINOPT_H_
#define _WINOPT_H_

/* Sybols needed by winopt */

extern HWND hMainWnd;
extern HINSTANCE hInst;
extern HMENU hMenu;
extern int major_version, minor_version;
extern char version[];
extern char title[];
extern char *program_name;
extern char *defined;
extern char *programhelpfile;

extern void win32_message(char *msg);
extern void win32_error_init(int i);
extern void win32_error(int line, char *message);
extern void win32_error2(int line, char *message, char *info);

extern void win32_log(char *msg);

extern void win32_fatal_error(int line, char *message);

extern int xpos, ypos; /* Window position */
extern int width,height; /* dimension of main window */


extern void set_option(char **option, char *str);
#define MAXARG 256
extern int mk_argv(char *argv[MAXARG],char *command, int unquote);

/* Symbols provided by winopt */
#include "uint64.h"


/* in winabout.c */
extern INT_PTR CALLBACK    
AboutDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );

/* in winconnect.c */
extern INT_PTR CALLBACK    
ConnectDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );
/* in windialog.c */
extern void register_subwindow(HWND h);
extern void unregister_subwindow(HWND h);
extern BOOL do_subwindow_msg(MSG *msg);

 
/* from winxy.c */
extern void set_xypos(HWND hWnd);
extern void get_xypos(void);

extern HBITMAP hBmp;
extern HBITMAP hon,hoff,hconnect;


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

/* from layout.c */

extern HFONT hFixedFont;
extern HFONT hVarFont;
extern int fixed_line_height;
extern int fixed_char_width;
extern int fixed_char_height;
extern int separator_width;
extern int separator_height; 
extern int version_width; /* length of the version string in VarFont */
extern int minimized;

#ifdef WIN32
extern void param_init(void);
#else
extern void param_init(int argc, char *argv[]);
#endif



extern LRESULT CALLBACK 
OptWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); /* provided */

#include <stdio.h>
FILE *win32_fopen(char *filename, char *mode);
/* open fiename, look in the configPATH and programpath before giving up */

#endif