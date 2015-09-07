#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"

HBITMAP hwait;

char version[]="$Revision: 1.3 $ $Date: 2015-09-07 15:50:50 $";

char howto[] =
"\n"
"The program will contact the motherboard at host:port\n"
"and register itself as MMIX CPU.\n"
"Then, the program will expect a connection from gdb over TCP/IP.\n"
"\n"
;
 void vmb_poweron(void)
{ 
  SendMessage(hMainWnd,WM_USER+1,0,0);
}

void vmb_poweroff(void)
{  
   SendMessage(hMainWnd,WM_USER+2,0,0);
}

void vmb_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{ 
   PostMessage(hMainWnd,WM_CLOSE,0,0);
}

void vmb_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ /* do nothing */
   SendMessage(hMainWnd,WM_USER+4,0,0);
}



INT_PTR CALLBACK  
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{

  switch ( message )
  { case WM_INITDIALOG:
#if 0
      uint64_to_hex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
      SetDlgItemText(hDlg,IDC_FILE,filename);
#endif
      return TRUE;
   case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
#if 0
      if( wparam == IDOK )
      { GetDlgItemText(hDlg,IDC_ADDRESS,tmp_option,MAXTMPOPTION);
        vmb_address = strtouint64(tmp_option);
        GetDlgItemText(hDlg,IDC_FILE,tmp_option,MAXTMPOPTION);
	    set_option(&filename,tmp_option);
		open_file();
      }
	  else if (HIWORD(wparam) == BN_CLICKED  && LOWORD(wparam) == IDC_BROWSE) 
	  { OPENFILENAME ofn;       /* common dialog box structure */
         /* Initialize OPENFILENAME */
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hMainWnd;
        ofn.lpstrFile = tmp_option;
        ofn.nMaxFile = MAXTMPOPTION;
        ofn.lpstrFilter = "All\0*.*\0Rom\0*.rom\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        /* Display the Open dialog box. */
        if (GetOpenFileName(&ofn)==TRUE) 
		   SetDlgItemText(hDlg,IDC_FILE,tmp_option);
	  }
#endif
     if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
  }
  return FALSE;
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
 case WM_USER+5: /* waiting for gdb */
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hwait);
	return 0;
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}


extern DWORD WINAPI mmix_main(LPVOID lpParameter);


void init_device(void)
{ 
 
}



HINSTANCE hInst;
HWND hMainWnd;
HBITMAP hBmp;
HMENU hMenu;
HBITMAP hon,hoff,hconnect;


BOOL InitInstance(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;
  BITMAP bm;
  int r;

#define MAX_LOADSTRING 100		
  static TCHAR szClassName[MAX_LOADSTRING];
  static TCHAR szTitle[MAX_LOADSTRING];
  hInst = hInstance; 

  r = LoadString(hInstance, IDS_CLASS, szClassName, MAX_LOADSTRING);
  if (r==0)
  { r = GetLastError();
    vmb_debugi(1,"Unable to load class name (%X)",r);
	vmb_fatal_error(__LINE__,"Unable to load class name");
  }
  r = LoadString(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);
  if (r==0)
  { r = GetLastError();
    vmb_debugi(1,"Unable to load window title (%X)",r);
  }
  ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; /*(HBRUSH)(COLOR_WINDOW+1);*/
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
/*	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);
*/
	if (!RegisterClassEx(&wcex)) return FALSE;

	GetObject(hBmp, sizeof(bm), &bm);

    hMainWnd = CreateWindow(szClassName, szTitle ,WS_POPUP,
                            xpos, ypos, bm.bmWidth, bm.bmHeight,
	                        NULL, NULL, hInstance, NULL);

    if (hMainWnd) 
	{ 
	  HRGN h = BitmapToRegion(hBmp);
	  if (h) SetWindowRgn(hMainWnd, h, TRUE);
	}

   return TRUE;
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HACCEL hAccelTable;
    MSG msg;

	hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));
	hBmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BITMAP), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hon = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_ON), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
    hconnect = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_CONNECT), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
    hoff = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OFF), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
	hwait = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_WAIT), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);

	if (hBmp==NULL) return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	if (!InitInstance (hInstance)) return FALSE;
	
	param_init();
    get_pos_key(&xpos,&ypos,defined);
	{ DWORD dwMMIXThreadId;
      HANDLE hMMIXThread;
      hMMIXThread = CreateThread( 
            NULL,              // default security attributes
            0,                 // use default stack size  
            mmix_main,        // thread function 
            NULL,             // argument to thread function 
            0,                 // use default creation flags 
            &dwMMIXThreadId);   // returns the thread identifier 
        // Check the return value for success. 
      if (hMMIXThread == NULL) 
        vmb_fatal_error(__LINE__, "Creation of mmix thread failed");
      /* in the moment, I really dont use the handle */
      CloseHandle(hMMIXThread);
	}

	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
	UpdateWindow(hMainWnd);

    SendMessage(hMainWnd,WM_USER+3,0,0); /* the connect button */
	if (vmb_debug_flag) vmb_debug_on(); else vmb_debug_off();
	CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|(vmb_debug_flag?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_VERBOSE,MF_BYCOMMAND|(vmb_verbose_level==0?MF_CHECKED:MF_UNCHECKED));

	while (GetMessage(&msg, NULL, 0, 0)) 
	  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	vmb_disconnect();
    set_pos_key(xpos,ypos,defined);
	return (int)msg.wParam;
}