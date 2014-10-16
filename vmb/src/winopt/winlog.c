#include <windows.h>
#include <commctrl.h>
#include "vmb.h"
#include "resource.h"
#include "winopt.h"
#include "inspect.h"


HWND hLog=NULL; /* logging output goes to this window, if not NULL */
static WNDPROC StaticWndProc=NULL;
static HFONT hLogFont=NULL;
#define WM_VMB_MSG			(WM_APP+1)


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
/* we make the msg_buffer into a string and return its length*/
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

void win32_log(char *msg)
{ if (hLog == NULL) return;
  if (*msg == 0) return;
  EnterCriticalSection (&msg_section);
  while (*msg)
  { if (msg[0]=='\r' && msg[1]=='\n')
    { add_msg_char(*msg); msg++; 
    }
	else if (msg[0]=='\n') 
	  add_msg_char('\r');
	add_msg_char(*msg);
	msg++;
  }
  if (msg_changed==0)
    PostMessage(hLog,WM_VMB_MSG,0,0);
  msg_changed++;
  LeaveCriticalSection (&msg_section);
}


extern LRESULT CALLBACK   
LogWndProc( HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam )
{ 
	if ( message== WM_VMB_MSG )
	{ int n; 
	  EnterCriticalSection (&msg_section);
	  n = normalize_msg();
	  msg_changed = 0;
	  CallWindowProc(StaticWndProc,hWnd,WM_SETTEXT,0,(LPARAM)msg_buffer);
	  LeaveCriticalSection (&msg_section);
      SendMessage(hWnd,EM_SETSEL,n,n);
      SendMessage(hWnd,EM_SCROLLCARET,0,0); 
	  return 0;
	}
	else if (message == WM_DESTROY) 
	{ unregister_subwindow(hLog);
	  DeleteCriticalSection(&msg_section);
	  DeleteObject(hLogFont);
	  hLogFont=NULL;
	  hLog=NULL;
	}
	return CallWindowProc(StaticWndProc, hWnd, message, wparam,
                         lparam);
}

#include "richedit.h"
HWND CreateLog(HWND hParent,HINSTANCE hInst)
{ 
  InitializeCriticalSection (&msg_section);
	hLog = CreateWindow("EDIT",
						NULL,WS_CHILD|WS_VISIBLE|WS_BORDER|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL|ES_READONLY,
						0,0,100,10,
						hParent,NULL,hInst,0);
    StaticWndProc = (WNDPROC)SetWindowLongPtr(hLog, GWLP_WNDPROC,(LONG)(LONG_PTR)LogWndProc);
	hLogFont=GetStockObject(ANSI_FIXED_FONT); 
#if 0
	{ int nHeight;
      HDC hdc;
      int ydpi;
	   hdc =GetDC(NULL);
	   ydpi = GetDeviceCaps(hdc, LOGPIXELSY);
	   ReleaseDC(NULL,hdc);
	   nHeight = MulDiv(8,ydpi , 72);
	   hLogFont = CreateFont(-nHeight,0,0,0,FW_REGULAR,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,FIXED_PITCH|FF_MODERN,"MS Sans Serif");
	}
#endif

    SendMessage(hLog,WM_SETFONT,(WPARAM)hLogFont,0);
	register_subwindow(hLog);
    return hLog;
}

