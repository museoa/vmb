#include <windows.h>
#include "dedit.h"
#include "winde.h"



#define MAXDATAEDIT 4
static HWND hDataEdit[MAXDATAEDIT] = {0};
static ATOM hDataEditClass=0;
HINSTANCE hDataEditInstance=0;
HWND hDataEditParent=0;


static LRESULT CALLBACK DataEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch ( message )
  { case WM_CREATE :
    { RECT rect;
	  HWND hDataEdit = CreateDataEdit(hDataEditInstance,hWnd);
	  SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG)(LONG_PTR)hDataEdit);
      GetWindowRect(hDataEdit,&rect);
	  AdjustWindowRect(&rect,WS_POPUP|WS_SYSMENU|WS_CAPTION|WS_VISIBLE,FALSE);
	  SetWindowPos(hWnd,HWND_TOP,0,0,rect.right-rect.left,rect.bottom-rect.top,
		  SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW);
      UpdateWindow(hWnd);
	}
  	 return 0;
	case WM_CLOSE:
	{ int id;
	  HWND child = (HWND)(LONG_PTR)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	  de_connect(child,NULL);
      for (id=0;id<MAXDATAEDIT;id++)
	    if (hDataEdit[id]==child)
           hDataEdit[id]=NULL;
      }
	  DestroyWindow(hWnd);
	  return 0;
 }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

static ATOM RegisterDataEditClass(HINSTANCE hInst)
{ WNDCLASSEX wcex;
  ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)DataEditProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; /*(HBRUSH)(COLOR_WINDOW+1);*/
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName = "DATAEDITCLASS";
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

HWND GetDataEdit(int id, HWND hMemory)
{ HWND h;
  RECT rect;
  if (id <0 || id>=MAXDATAEDIT) 
  { vmb_error(__LINE__,"ID out of range in GetDataEdit");
    id = 0;
  }
  if (hDataEdit[id]!=NULL)
    return hDataEdit[id];
  if (hDataEditClass==0)
    hDataEditClass=RegisterDataEditClass(hDataEditInstance);
  if (hDataEditParent!=NULL)
    GetWindowRect(hDataEditParent,&rect);
  else
    rect.left=rect.bottom=CW_USEDEFAULT;
  h = CreateWindow("DATAEDITCLASS","Data Editor", WS_OVERLAPPED|WS_SYSMENU|WS_CAPTION|WS_VISIBLE,
			rect.left,rect.bottom,CW_USEDEFAULT,CW_USEDEFAULT,
		  hDataEditParent,NULL,hDataEditInstance,0);
  if (h!=NULL)
    hDataEdit[id]=(HWND)(LONG_PTR)GetWindowLongPtr(h,GWLP_USERDATA);
  if (hDataEdit[id]==NULL)
    vmb_fatal_error(__LINE__,"Unable to create Data Editor");
  return hDataEdit[id];
}

void DestroyDataEdit(int id)
{ if (hDataEdit[id]!=NULL)
  { DestroyWindow(GetParent(hDataEdit[id]));
    hDataEdit[id]=NULL;
  }
}


