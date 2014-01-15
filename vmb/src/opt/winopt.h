#include "vmb.h"
#include "winres.h"

extern HWND hMainWnd;
extern HINSTANCE hInst;
extern HBITMAP hBmp;
extern HMENU hMenu;
extern HBITMAP hon,hoff,hconnect;
extern HWND hpower;
extern device_info vmb;

extern void win32_message(char *msg);
extern void win32_debug(char *msg);
extern void win32_error_init(int i);
extern void mem_update(unsigned int offset, int size);
/* call this function to tell a specific memory inspector i that an update is due */
extern void mem_update_i(int i, unsigned int offset, int size);

extern HWND hDebug;
extern void init_device(device_info *vmb);
extern INT_PTR CALLBACK   
DebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );

extern HRGN BitmapToRegion (HBITMAP hBmp);


extern int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow);
                     
extern LRESULT CALLBACK 
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); /*needed */

extern INT_PTR CALLBACK   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam ); /*needed */


extern LRESULT CALLBACK 
OptWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); /* provided */

extern INT_PTR CALLBACK    
ConnectDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );
                     


extern INT_PTR CALLBACK    
AboutDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );

extern INT_PTR CALLBACK    
ConfigurationDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );
                  
extern int set_reg_DWORD(char *program, char *name, DWORD value);
extern DWORD get_reg_DWORD(char *program, char *name);

extern void set_xypos(HWND hWnd);
extern void get_xypos(void);
extern void set_pos_key(HWND hWnd, char *name);
extern void get_pos_key(int *Xpos, int *Ypos, char *name);

extern void register_subwindow(HWND h);
extern void unregister_subwindow(HWND h);
extern BOOL do_subwindow_msg(MSG *msg);

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

extern void write_regtab(char *program);
extern void read_regtab(char * program);