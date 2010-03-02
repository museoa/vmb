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

                     
extern void set_pos_key(HWND hWnd,char *name);
extern void get_pos_key(int *Xpos, int *Ypos, char *name);


