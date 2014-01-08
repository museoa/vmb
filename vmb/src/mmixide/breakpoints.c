#include <stdio.h>
#include <windows.h>
#include "vmb.h"
#include "error.h"
#include "splitter.h"
#include "winmain.h"
#include "info.h"
#include "resource.h"
#include "breakpoints.h"


#if 0
void breaks_list_reset(void)
{  if (hFileList!=NULL)
	SendMessage(hFileList,LB_RESETCONTENT,0,0);	
}



#define MAX_LISTNAME 64
char *breaks_listname(int breaks_no)
{ static char listname[MAX_LISTNAME+7];
  static char noname[]="Unnamed";
  char *name = breaks2shortname(breaks_no);
  if (name==NULL) name=noname;
  if (breaks_no==0) return name;
  if(unique_shortname(breaks_no)) return name;
  sprintf_s(listname,MAX_LISTNAME,"%.64s (%d)",name,breaks_no);
  return listname;
}

void breaks_list_remove(int breaks_no)
{ int item;
  char *name;
  if (hFileList==NULL) return;
  name = breaks_listname(breaks_no);
  item = (int)SendMessage(hFileList,LB_FINDSTRINGEXACT,-1,(LPARAM)name);
  if(item==LB_ERR) return;
  SendMessage(hFileList,LB_DELETESTRING,item,0);
}

void breaks_list_add(int breaks_no)
{ LPARAM item;
  char *name;
  if (hFileList==NULL) return;
  name = breaks_listname(breaks_no);
  item = (int)SendMessage(hFileList,LB_FINDSTRINGEXACT,-1,(LPARAM)name);
  if(item!=LB_ERR) return;
  item = SendMessage(hFileList,LB_ADDSTRING,0,(LPARAM)name);
  SendMessage(hFileList,LB_SETITEMDATA,item,breaks_no);
}

void breaks_list_mark(int breaks_no)
{ int item;
  char *name;
  if (hFileList==NULL) return;
  name = breaks_listname(breaks_no);
  item = (int)SendMessage(hFileList,LB_FINDSTRINGEXACT,-1,(LPARAM)name);
  if(item==LB_ERR) return;
  SendMessage(hFileList,LB_SETCURSEL,item,0);
}

void create_breakslist(void)
{ sp_create_options(1,1,0.15,0,hEdit);
  hFileList = CreateWindow("LISTBOX","File List",
		     WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_SORT|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED,
             0,0,0,0,
	         hSplitter, NULL, hInst, NULL);
  for_all_breakss(breaks_list_add);
  breaks_list_mark(edit_breaks_no);
}

int breaks_list_measureitem(LPMEASUREITEMSTRUCT lm)
{   lm->itemHeight = 20; 
	return 1;
}

extern int breaks_list_drawitem(LPDRAWITEMSTRUCT di)
{ int len;
  char *str;
  TEXTMETRIC tm; 
  int y; 
  COLORREF cr;
  if (di->itemID <0 ) return 0;
  switch (di->itemAction) 
  { case ODA_SELECT: 
    case ODA_DRAWENTIRE: 
      len = (int)SendMessage(di->hwndItem, LB_GETTEXTLEN, di->itemID, 0); 
	  str = malloc(len+1);
	  if (str==NULL) 
	  { vmb_error(__LINE__,"Out of memory");
	    return 0;
	  }
      SendMessage(di->hwndItem, LB_GETTEXT, di->itemID, (LPARAM)str); 
      GetTextMetrics(di->hDC, &tm); 
      y = (di->rcItem.bottom + di->rcItem.top - tm.tmHeight) / 2;
      if (di->itemState & ODS_SELECTED)
		  cr = SetBkColor(di->hDC,RGB(0x80,0x80,0xff));
	  else
		  cr =SetBkColor(di->hDC,RGB(0xff,0xff,0xff));
	  ExtTextOut(di->hDC, 15, y,ETO_CLIPPED|ETO_OPAQUE, &di->rcItem,str,len,NULL);
	  SetBkColor(di->hDC,cr);
	  free(str);

    return 1;
  } 
  return 0;
}
#endif

static int listx, listy;
static void resize(HWND hDlg, int w, int h)
{ 
  MoveWindow(GetDlgItem(hDlg,IDC_LIST_BREAKPOINTS),listx,listy,w-2*listx,h-listy-listx,TRUE);
}

INT_PTR CALLBACK    
BreakpointsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      { RECT r,d;
	  	GetWindowRect(hDlg,&r);
	    GetWindowRect(GetDlgItem(hDlg,IDC_LIST_BREAKPOINTS),&d);
		listx=d.left-r.left;
		listy=d.top-r.top;
		resize(hDlg,r.right-r.left,r.bottom-r.top);
      }
      return TRUE;
  case WM_CLOSE:
      hBreakpoints=NULL;
	  DestroyWindow(hDlg);
	  return TRUE;
  case WM_SIZE:
	  resize(hDlg,LOWORD(lparam),HIWORD(lparam));
	  return TRUE;
    case WM_COMMAND:
      if (wparam==IDC_ADD)
	  {
        return TRUE;
	  }
     break;
  }
  return FALSE;
}


 HWND hBreakpoints=NULL;

 void create_breakpoints(void)
 { if (hBreakpoints!=NULL) return;
   sp_create_options(0,1,0.3,0,NULL);
   hBreakpoints = CreateDialog(hInst,MAKEINTRESOURCE(IDD_BREAKPOINTS),hSplitter,BreakpointsDialogProc); 
 }