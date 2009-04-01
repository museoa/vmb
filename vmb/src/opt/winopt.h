extern HWND hMainWnd;
extern HINSTANCE hInst;
extern HBITMAP hBmp;
extern HMENU hMenu;
extern HBITMAP hon,hoff,hconnect;
extern HWND hpower;

extern void win32_message(char *msg);
extern void win32_debug(char *msg);
extern HWND hDebug;
extern void init_device(void);
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


#define IDS_CLASS                       1
#define IDS_TITLE                       2

#define IDB_BITMAP                      101

#define IDR_MENU                        104

#define IDR_ACCELERATOR                 106
#define IDI_ICON                        107

#define IDB_CONNECT                     114
#define IDB_OFF                         115

#define IDB_ON                          119


#define IDD_ABOUT                       121            
#define IDD_CONNECT                     122
#define IDD_DEBUG                       123
#define IDD_SETTINGS                    124

#define IDC_THE_SERVER                  1001
#define IDC_THE_PORT                    1002
#define IDC_DEBUG                       1003

#define ID_CONNECT                      40007
#define ID_DEBUG                        40008
#define ID_VERBOSE                      40009
#define ID_EXIT                         40012
#define ID_ABOUT                        40013
#define ID_HELP_ABOUT                   40014
#define ID_SETTINGS                     40015
#define ID_MINIMIZE                     40016


