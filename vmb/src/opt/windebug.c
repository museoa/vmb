#include <windows.h>
#include <commctrl.h>
#include "vmb.h"
#include "resource.h"
#include "winopt.h"
#include "inspect.h"

void win32_message(char *msg)
{
	MessageBox(NULL,msg,"Message",MB_OK);
}

HWND hDebug=NULL; /* debug output goes to this window, if not NULL */
HWND hFilter=NULL; 

#define MAX_MSG_BUFFER	0x10000
#define MAX_MSG_FILL (MAX_MSG_BUFFER/2)
#define MSG_BUFFER_MASK (MAX_MSG_BUFFER-1)
#define MSG_FILL ((msg_end-msg_begin)&MSG_BUFFER_MASK)
static char msg_buffer[MAX_MSG_BUFFER];
static int msg_begin, msg_end = 0;
static int msg_changed = 0;
static CRITICAL_SECTION   msg_section;

static void add_msg_char(char c)
{ if (MSG_FILL>=MAX_MSG_FILL) /* we keep it halve empty*/
  { int n;
    char c;
    n = MAX_MSG_FILL/16;
	c = 0;
	do { /* remove 1/16 of the buffer or one line */
      c = msg_buffer[msg_begin];
	  msg_begin = (msg_begin+1)&MSG_BUFFER_MASK; /*make space */
	  n--;
	} while (n>0 && c!='\n');
  }
  msg_buffer[msg_end] = c;
  msg_end = (msg_end+1)&MSG_BUFFER_MASK;
}

static int normalize_msg(void)
/* we make the msg_buffer into a string return its length*/
{ if (msg_begin!=0) 
  { if (msg_end>msg_begin)
    { memmove(msg_buffer,msg_buffer+msg_begin,msg_end-msg_begin);
      msg_end = msg_end-msg_begin;
	  msg_begin = 0;
    }
    else if (msg_end<msg_begin) /* split buffer */
    { int head, tail;
      head = MAX_MSG_BUFFER-msg_begin;
	  tail = msg_end;
	  memmove(msg_buffer+head,msg_buffer,tail);
	  memmove(msg_buffer,msg_buffer+msg_begin,head);
      msg_end = head+tail;
	  msg_begin = 0;
    }
    else
      msg_begin=msg_end=0;
  }
  msg_buffer[msg_end] = 0; /* trailing zero */
  return msg_end;
}

void win32_debug(char *msg)
{ if (hDebug == NULL) return;
  if (*msg == 0) return;
  EnterCriticalSection (&msg_section);
  while (*msg)
  { add_msg_char(*msg);
	msg++;
  }
  add_msg_char('\r');
  add_msg_char('\n');
  if (msg_changed==0)
    PostMessage(hDebug,WM_VMB_MSG,0,0);
  msg_changed++;
  LeaveCriticalSection (&msg_section);
}

void show_filter(void)
{ CheckDlgButton(hFilter,IDC_HIDE_FATAL,(vmb_debug_mask&VMB_DEBUG_FATAL)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hFilter,IDC_HIDE_ERROR,(vmb_debug_mask&VMB_DEBUG_ERROR)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hFilter,IDC_HIDE_WARN,(vmb_debug_mask&VMB_DEBUG_WARN)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hFilter,IDC_HIDE_NOTIFY,(vmb_debug_mask&VMB_DEBUG_NOTIFY)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hFilter,IDC_HIDE_PROGRESS,(vmb_debug_mask&VMB_DEBUG_PROGRESS)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hFilter,IDC_HIDE_INFO,(vmb_debug_mask&VMB_DEBUG_INFO)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hFilter,IDC_HIDE_MSG,(vmb_debug_mask&VMB_DEBUG_MSG)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hFilter,IDC_HIDE_PAYLOAD,(vmb_debug_mask&VMB_DEBUG_PAYLOAD)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hFilter,IDC_HIDE_ALL,vmb_debug_mask==0?BST_UNCHECKED:
		  ((vmb_debug_mask&0xFF)==0xFF?BST_CHECKED:BST_INDETERMINATE));
}


void resize_dialog(HWND hDlg,int w, int h)
{ MoveWindow(GetDlgItem(hDlg,IDC_TAB_DEBUG),5,5,w-10,h-10,TRUE); 
  MoveWindow(GetDlgItem(hDlg,IDC_DEBUG),10,30,w-20,h-40,TRUE); 
  MoveWindow(hMemory,10,30,w-20,h-40,TRUE);
  MoveWindow(hFilter,10,30,w-20,h-40,TRUE);
}


INT_PTR CALLBACK FilterProc(HWND hDlg,UINT uMsg,WPARAM wparam,LPARAM lParam)
{ switch ( uMsg )
  { case WM_COMMAND: 
	if (HIWORD(wparam) == BN_CLICKED) 
     { int flag = 0;
	   switch (LOWORD(wparam)) 
	   { case IDC_HIDE_FATAL: flag = VMB_DEBUG_FATAL; break;
		 case IDC_HIDE_ERROR: flag = VMB_DEBUG_ERROR; break;	
		 case IDC_HIDE_WARN: flag = VMB_DEBUG_WARN; break;	
		 case IDC_HIDE_NOTIFY: flag = VMB_DEBUG_NOTIFY; break;	
		 case IDC_HIDE_PROGRESS: flag = VMB_DEBUG_PROGRESS; break;	
		 case IDC_HIDE_INFO: flag = VMB_DEBUG_INFO; break;	
		 case IDC_HIDE_MSG: flag = VMB_DEBUG_MSG; break;	
		 case IDC_HIDE_PAYLOAD: flag = VMB_DEBUG_PAYLOAD; break;	
		 case IDC_HIDE_ALL:
		 { LRESULT bs; 
		   bs = SendDlgItemMessage(hDlg, IDC_HIDE_ALL, BM_GETSTATE, 0, 0);
		   if (bs & BST_CHECKED) vmb_debug_mask = 0xFFFF;
		   else if (bs & BST_INDETERMINATE) vmb_debug_mask = VMB_DEBUG_DEFAULT;
		   else vmb_debug_mask = 0;
		   show_filter();
	       return FALSE;
		 }
	   }
       /* Retrieve the state of the check box. */
       if (SendDlgItemMessage(hDlg, LOWORD(wparam), BM_GETSTATE, 0, 0)&BST_CHECKED)
             vmb_debug_mask |= flag;
		   else
             vmb_debug_mask &= ~flag;
       return FALSE; 
	 }
     break;
  }
return FALSE;
}
	

INT_PTR CALLBACK   
DebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ static int minw, minh;
  switch ( message )
  { case WM_INITDIALOG :
  	InitializeCriticalSection (&msg_section);
	hMemory=CreateDialog(hInst,MAKEINTRESOURCE(IDD_MEMORY),hDlg,MemoryDialogProc);
	register_subwindow(hMemory);
	hFilter=CreateDialog(hInst,MAKEINTRESOURCE(IDD_FILTER),hDlg,FilterProc);
    register_subwindow(hFilter);
	hDebug=hDlg;
	register_subwindow(hDebug);
	{ TCITEM tie;
	  RECT rect;
	  int i=0;
      tie.mask = TCIF_TEXT;
      tie.iImage = -1;
 	  tie.pszText = "Debug";
	  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_TAB_DEBUG), 0, &tie);
 	  tie.pszText = "Filter";
	  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_TAB_DEBUG), 1, &tie);
	  while (inspector[i].name!=NULL)
	  { tie.pszText = inspector[i].name;
	    TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_TAB_DEBUG), i+2, &tie);
		i++;
	  }

	  TabCtrl_SetCurSel (GetDlgItem (hDlg, IDC_TAB_DEBUG), 0);
	  ShowWindow(GetDlgItem(hDlg,IDC_DEBUG),SW_SHOW);
	  GetWindowRect(hDlg,&rect);
      minw = rect.right-rect.left;
	  minh = rect.bottom-rect.top;
	  GetClientRect(hDlg,&rect);
      resize_dialog(hDlg,rect.right-rect.left,rect.bottom-rect.top);
	}
    return FALSE; /* no keyboard focus on the debug window */
  case WM_NOTIFY:
	{ NMHDR *p;
      p = (NMHDR*)lparam;
      if (p->code == TCN_SELCHANGE) 
      { int i = TabCtrl_GetCurSel (GetDlgItem (hDlg, IDC_TAB_DEBUG));
	    ShowWindow(GetDlgItem(hDlg,IDC_DEBUG),i==0?SW_SHOW:SW_HIDE);
        ShowWindow(hFilter,i==1?SW_SHOW:SW_HIDE);
	    if (i==1) show_filter();
        ShowWindow(hMemory,i>1?SW_SHOW:SW_HIDE); 
		if (i>1) {
			insp=i-2;
			adjust_memory_tab();
		}
		else insp=0;
      }
	}
    break;
   case WM_VMB_MSG:
	    if (hDebug!=NULL)
		{ int n; 
		  EnterCriticalSection (&msg_section);
		  n = normalize_msg();
		  msg_changed = 0;
		  SetDlgItemText(hDlg,IDC_DEBUG,msg_buffer);
		  LeaveCriticalSection (&msg_section);
		  /* n = SendDlgItemMessage(hDlg,IDC_DEBUG,WM_GETTEXTLENGTH,0,0); */
          SendDlgItemMessage(hDlg,IDC_DEBUG,EM_SETSEL,n,n);
          SendDlgItemMessage(hDlg,IDC_DEBUG,EM_SCROLLCARET,0,0); 
		}
    return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { unregister_subwindow(hFilter);
	    DestroyWindow(hFilter);
	    unregister_subwindow(hMemory);
		DestroyWindow(hMemory);
	    CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|MF_UNCHECKED);
		unregister_subwindow(hDebug);
		hDebug = NULL;
		EndDialog(hDlg, TRUE);
		DeleteCriticalSection(&msg_section);
        return TRUE;
      }
      break;
    case WM_SIZE: 
	  resize_dialog(hDlg,LOWORD(lparam),HIWORD(lparam));
      return TRUE;
	case WM_GETMINMAXINFO:
		{ MINMAXINFO *p = (MINMAXINFO *)lparam;
		  p->ptMinTrackSize.x = minw;
          p->ptMinTrackSize.y = minh;
		}
		return 0;
  }
  return FALSE;
}

void win32_error_init(int on)
{ if (on)
  { if (hDebug==NULL)
      hDebug=CreateDialog(hInst,MAKEINTRESOURCE(IDD_DEBUG),hMainWnd,DebugDialogProc);
	SetWindowText(hDebug,defined);
	CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|MF_CHECKED);
  }
  else
  { if (hDebug!=NULL)
      SendMessage(hDebug,WM_SYSCOMMAND,SC_CLOSE,0);
	CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|MF_UNCHECKED);
  }
}