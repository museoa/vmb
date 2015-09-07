#ifndef _OPT_H_
#define _OPT_H_

#include "vmb.h"
#include "winres.h"


extern device_info vmb;

extern HWND hpower;


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
ConfigurationDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );


#endif
