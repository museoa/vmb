#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "winopt.h"
#include "inspect.h"


HWND hLog=NULL; /* logging output goes to this window, if not NULL */
static WNDPROC StaticWndProc=NULL;
static HFONT hLogFont=NULL;
#define WM_VMB_MSG			(WM_APP+1)
#define WM_VMB_GETS			(WM_APP+2)


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

static HANDLE hGets=NULL;
extern char stdin_buf[256]; /* standard input to the simulated program */
extern char *stdin_buf_start; /* current position in that buffer */
extern char *stdin_buf_end; /* current end of that buffer */
static int loginput=0;

static UINT mes[10009]={0};
static int mesc=0;

extern LRESULT CALLBACK   
LogWndProc( HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch(message)
  { case WM_VMB_MSG:
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
	case WM_VMB_GETS:
	  SetFocus(hWnd);
	  ShowCaret(hWnd);
	  loginput=1;
	  mesc=0;
	  return 0;
	case WM_DESTROY:  
	  DeleteCriticalSection(&msg_section);
	  DeleteObject(hLogFont);
      CloseHandle(hGets); 
      hGets=NULL;
	  hLogFont=NULL;
	  hLog=NULL;
	  break;
	case WM_CHAR:
	{ if (!loginput) return 0;
	  else if (wparam == VK_RETURN) 
	   *stdin_buf_end++='\n';
	  else if (wparam == VK_BACK)
	  { if (stdin_buf_end> stdin_buf) 
	    { stdin_buf_end--;
	      break;
	    }
	    else
		  return 0;
	  }
	  else if (stdin_buf_end<stdin_buf+254)
	  { *stdin_buf_end++=(char)wparam;
	    if (wparam<0x20) wparam=0x7F;
	    if (stdin_buf_end<stdin_buf+254) break;
	  }
      *stdin_buf_end=0;
	  loginput=0;
	  HideCaret(hWnd);
	  SetEvent (hGets);	 
      break;
	}
	case WM_KEYDOWN:
		if (loginput)
		{ if (wparam==VK_RETURN || wparam==VK_BACK) break;
		  else	if (wparam <=VK_HELP)
		    return 0; 
		}
	case WM_MOUSEACTIVATE: 
	case WM_NCHITTEST: 
    case WM_SETCURSOR:
      return CallWindowProc(StaticWndProc, hWnd, message, wparam, lparam);
  }
    
  if (loginput)
  { if (message==WM_MOUSEMOVE) return 0;
    else if (message  >=WM_MOUSEFIRST && message <=WM_MOUSELAST)
	{ SetFocus(hWnd); return 0;}
	else 
	  mes[mesc++]=message;
  }
  return CallWindowProc(StaticWndProc, hWnd, message, wparam, lparam);
}

#include "richedit.h"
HWND CreateLog(HWND hParent,HINSTANCE hInst)
{ 
  InitializeCriticalSection (&msg_section);
	hLog = CreateWindow("EDIT",
						NULL,WS_CHILD|WS_VISIBLE|WS_BORDER|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL, //|ES_READONLY,
						0,0,100,10,
						hParent,NULL,hInst,0);
    StaticWndProc = (WNDPROC)SetWindowLongPtr(hLog, GWLP_WNDPROC,(LONG)(LONG_PTR)LogWndProc);
	hLogFont=GetStockObject(ANSI_FIXED_FONT); 
	hGets=CreateEvent(NULL,FALSE,FALSE,NULL);
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
    return hLog;
}

char stdin_chr(void)
{
	while(stdin_buf_start==stdin_buf_end){
		win32_log("StdIn> ");
        //win32_caret(1);
	    stdin_buf_start= stdin_buf_end=stdin_buf;
		PostMessage(hLog,WM_VMB_GETS,0,0);
        WaitForSingleObject(hGets,INFINITE);
		win32_log(stdin_buf);
	}
    //win32_caret(0);
	return*stdin_buf_start++;
}
