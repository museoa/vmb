#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "param.h"
#include "option.h"



/* Global Variables loaded form the Resourcefile*/
#define MAX_LOADSTRING 100			
static TCHAR szClassName[MAX_LOADSTRING];
static TCHAR szTitle[MAX_LOADSTRING];
static HBITMAP hBmp;
static HMENU hMenu;
static HBITMAP hon,hoff,hconnect;

/* Global Variables for important Windows */
static HWND hpower;
static HWND hDebug=NULL; /* debug output goes to this window, if not NULL */
HWND hMainWnd;
static HINSTANCE hInst;

/* Global variables defining the properties of the device */
#define MAXHOST 1024
unsigned int address_hi=0;
unsigned int address_lo=0;

void win32_message(char *msg)
{
	MessageBox(NULL,msg,"Message",MB_OK);
}

void win32_debug(char *msg)
{ static char nl[] ="\r\n";	
  LRESULT  n;
  if (hDebug == NULL) return;
  n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_GETLINECOUNT,0,0);
  if (n>100)
  { n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_LINELENGTH,0,0);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,0,n+2);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)"");
    n = SendDlgItemMessage(hDebug,IDC_DEBUG,WM_GETTEXTLENGTH,0,0);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,n,n);
  }
  SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)msg);
  SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)nl);
}


INT_PTR CALLBACK   
DebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { vmb_debug_flag = 0;
	    hDebug = NULL;
		EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
	case WM_SIZE: 
      MoveWindow(GetDlgItem(hDebug,IDC_DEBUG),5,5,LOWORD(lparam)-10,HIWORD(lparam)-10,TRUE); 
       return TRUE;
    break;
  }
  return FALSE;
}

INT_PTR CALLBACK    
AboutDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_INITDIALOG:
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      {
        EndDialog(hDlg, TRUE);
        return TRUE;
      }
     break;
  }
  return FALSE;
}




INT_PTR CALLBACK    
ConnectDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{

  switch ( message )
  { case WM_INITDIALOG:
      SetDlgItemText(hDlg,IDC_THE_SERVER,host);
      SetDlgItemInt(hDlg,IDC_THE_PORT,port,FALSE);
      return TRUE;
   case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
        { 
	      GetDlgItemText(hDlg,IDC_THE_SERVER,host,MAXHOST);
          port = GetDlgItemInt(hDlg,IDC_THE_PORT,NULL,FALSE);
		  if (!vmb_connected)
		  {  vmb_connect(host,port);
		     hextoint(hexaddress,&address_hi);
             hextoint(hexaddress+8,&address_lo);
		     vmb_register(address_hi, address_lo,size,0,0,defined);
			 SendMessage(hMainWnd,WM_USER+3,0,0); /* the connect button */
		  }
		  EndDialog(hDlg, TRUE);
          return TRUE;
      }
      else if( wparam == IDCANCEL )
      {
        EndDialog(hDlg, TRUE);
        return TRUE;
      }
     break;
  }
  return FALSE;
}





INT_PTR CALLBACK   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_INITDIALOG:
      { 
         SetDlgItemText(hDlg,IDC_ADDRESS,hexaddress);
	  }
      return TRUE;
   case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { GetDlgItemText(hDlg,IDC_ADDRESS,tmp_option,MAXTMPOPTION);
        set_option(&hexaddress, tmp_option);
        hextoint(hexaddress,&address_hi);
        hextoint(hexaddress+8,&address_lo);
      }
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
  case WM_NCHITTEST:
    return HTCAPTION;
  case WM_USER+1: /* Power On */
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon);
	return 0;
  case WM_USER+2: /* Power Off */
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
	return 0;
  case WM_USER+3: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
	SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
 	return 0;
  case WM_USER+4: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	   SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
	return 0;
  case WM_CREATE: 
	hpower = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,10,10,32,32,hWnd,NULL,hInst,0);
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
    return 0;
  case WM_PAINT:
    { PAINTSTRUCT ps;
      HDC hdc;
      hdc = BeginPaint (hWnd, &ps);
      if (hBmp)
      {	HDC memdc = CreateCompatibleDC(NULL);
        HBITMAP h = (HBITMAP)SelectObject(memdc, hBmp);
        BITMAP bm;
        GetObject(hBmp, sizeof(bm), &bm);
        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, memdc, 0, 0, SRCCOPY);
        SelectObject(memdc, h);
      }
      EndPaint (hWnd, &ps);
    }
    return 0;
  case WM_NCRBUTTONDOWN: /* right Mouse Button -> Context Menu */
    TrackPopupMenu(GetSubMenu(hMenu,0),TPM_LEFTALIGN|TPM_TOPALIGN,
		   LOWORD(lParam),HIWORD(lParam),0 ,hWnd,NULL);
    return 0;
  case WM_COMMAND:
    if (HIWORD(wParam)==0) /* Menu */
      switch(LOWORD(wParam))
	{ case ID_EXIT:
	    PostQuitMessage(0);
	    return 0;
	case ID_CONNECT:
	  if (!vmb_connected)
	    DialogBox(hInst,MAKEINTRESOURCE(IDD_CONNECT),hWnd,ConnectDialogProc);
	  else
	    vmb_disconnect();
	  return 0;
	case ID_SETTINGS:
	  DialogBox(hInst,MAKEINTRESOURCE(IDD_SETTINGS),hMainWnd,SettingsDialogProc);
	  return 0; 
	case ID_DEBUG:
	  if (hDebug==NULL)
	  {  vmb_debug_flag = 1;
	     hDebug= CreateDialog(hInst,MAKEINTRESOURCE(IDD_DEBUG),hWnd,DebugDialogProc);
	  }
	  return 0; 
	case ID_HELP_ABOUT:
	  DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,AboutDialogProc);
	  return 0; 
	case ID_HELP:
	  DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,AboutDialogProc);
	  return 0; 
	case ID_MINIMIZE:
	  CloseWindow(hWnd);
	  return 0; 
	}
    return 0;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  default:
    return (DefWindowProc(hWnd, message, wParam, lParam));
  }
 return (DefWindowProc(hWnd, message, wParam, lParam));
}


//
//	BitmapToRegion :	Create a region from the "non-transparent" pixels of a bitmap
//	Author :			Jean-Edouard Lachand-Robert (http://www.geocities.com/Paris/LeftBank/1160/resume.htm), June 1998.
//
//	hBmp :				Source bitmap
//	cTransparentColor :	Color base for the "transparent" pixels (default is black)
//	cTolerance :		Color tolerance for the "transparent" pixels.
//
//	A pixel is assumed to be transparent if the value of each of its 3 components (blue, green and red) is 
//	greater or equal to the corresponding value in cTransparentColor and is lower or equal to the 
//	corresponding value in cTransparentColor + cTolerance.
//
HRGN BitmapToRegion (HBITMAP hBmp)
{
	HRGN hRgn = NULL;

	if (hBmp)
	{
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);
		if (hMemDC)
		{
			// Get bitmap size
			BITMAP bm;
			BITMAPINFOHEADER RGB32BITSBITMAPINFO;
			VOID * pbits32; 
			HBITMAP hbm32; 
			
			GetObject(hBmp, sizeof(bm), &bm);

			ZeroMemory(&RGB32BITSBITMAPINFO, sizeof(RGB32BITSBITMAPINFO));

			RGB32BITSBITMAPINFO.biSize=sizeof(BITMAPINFOHEADER);
			RGB32BITSBITMAPINFO.biWidth=		bm.bmWidth;
			RGB32BITSBITMAPINFO.biHeight=		bm.bmHeight;
			RGB32BITSBITMAPINFO.biPlanes=		1;
			RGB32BITSBITMAPINFO.biBitCount=		32;

			RGB32BITSBITMAPINFO.biCompression=  BI_RGB;

            hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
		
			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory DC
				HDC hDC = CreateCompatibleDC(hMemDC);
				if (hDC)
				{
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits)
					BITMAP bm32;
					HBITMAP holdBmp;
					DWORD maxRects;
					HANDLE hData; 
					RGNDATA *pData;
					BYTE *p32,lr,lg,lb;
					HRGN h;
					int x,y;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy the bitmap into the memory DC
					holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// For better performances, we will use the ExtCreateRegion() function to create the
					// region. This function take a RGNDATA structure on entry. We will add rectangles by
					// amount of ALLOC_UNIT number in this structure.
					#define ALLOC_UNIT	100
					maxRects = ALLOC_UNIT;
					hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);


					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;

					// Keep on hand highest and lowest values for the "transparent" pixels
				    lr = GetRValue(*(LONG *)p32);
					lg = GetGValue(*(LONG *)p32);
					lb = GetBValue(*(LONG *)p32);

					for (y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to right
						for (x = 0; x < bm.bmWidth; x++)
						{
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if (b == lr)
								{
									b = GetGValue(*p);
									if (b == lg )
									{
										b = GetBValue(*p);
										if (b == lb )
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{   RECT *pr;
								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
								// large (ie: > 4000). Therefore, we have to create the region by multiple steps.
								if (pData->rdh.nCount == 2000)
								{
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									if (hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangles
					h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// Clean up
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}	
	}

	return hRgn;
}

BOOL InitInstance(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;
  BITMAP bm;

    hInst = hInstance; 
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
                            0, 0, bm.bmWidth, bm.bmHeight,
	                        NULL, NULL, hInstance, NULL);

    if (hMainWnd) 
	{ 
	  HRGN h = BitmapToRegion(hBmp);
	  if (h) SetWindowRgn(hMainWnd, h, TRUE);
	}

   return TRUE;
}



extern int ramsize;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HACCEL hAccelTable;
    MSG msg;
	LoadString(hInstance, IDS_CLASS, szClassName, MAX_LOADSTRING);
	LoadString(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);
	hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));
	hBmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BITMAP), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hon = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_ON), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
    hconnect = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_CONNECT), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
    hoff = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OFF), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
	if (hBmp==NULL) return FALSE;
 

	if (!InitInstance (hInstance)) return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);
	param_init();
    vmb_message_hook = win32_message;
	vmb_debug_hook = win32_debug;
	vmb_connect(host,port);
	hextoint(hexaddress,&address_hi);
    hextoint(hexaddress+8,&address_lo);
	vmb_register(address_hi,address_lo,size,0,0,defined);
    SendMessage(hMainWnd,WM_USER+3,0,0); /* the connect button */
	if (vmb_debug_flag)
	  SendMessage(hMainWnd,WM_COMMAND,(WPARAM)ID_DEBUG,0);

	while (GetMessage(&msg, NULL, 0, 0)) 
	  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	vmb_disconnect();
    return (int)msg.wParam;
}

