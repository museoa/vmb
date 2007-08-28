#include <windows.h>

extern void InitControlls(HINSTANCE hInst, HWND hWnd);

extern void PositionControlls(HWND hWnd,int width, int height);

extern void debug(char *msg);

extern void errormsg(char *message);

extern void power_led_position(int x, int y);

extern void get_settings(void);

extern void process_focus(int on);

extern HWND hMainWnd;

extern HWND hpower;

extern HWND hDebug;

extern HINSTANCE hInst;	

extern HBITMAP hon,hoff,hconnect, hBmp;



#define WM_SOCKET						(WM_USER+1)



/* the shared part of resource.h */



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



#define IDC_THE_SERVER                  1001

#define IDC_THE_PORT                    1002

#define IDC_DEBUG                       1003



#define ID_CONNECT                      40007

#define ID_DEBUG                        40008



#define ID_EXIT                         40012

#define ID_HELP_ABOUT                   40013



#define ID_SETTINGS                     40015

#define ID_MINIMIZE                     40016



