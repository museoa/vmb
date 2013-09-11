
#include <windows.h>
#include <afxres.h>
#include <stdio.h>
#include "winopt.h"
#include "resource.h"
#include "splitter.h"
#include "buttonbar.h"
#include "mmix-internals.h"
#include "debug.h"
#include "option.h"
#include "param.h"
#include "mmixrun.h"
#include "mmixlib.h"
#include "mmix-bus.h"
#include "winmain.h"

#define STATIC_BUILD
#include "../scintilla/include/scintilla.h"
#include "../scintilla/include/scilexer.h"
int major_version=1, minor_version=0;
char version[]="$Revision: 1.2 $ $Date: 2013-09-11 16:19:54 $";
char title[] ="VMB MMIX IDE";


sptr_t ed_send(unsigned int msg,uptr_t wparam,sptr_t lparam);
void ed_open(void);
void ed_close(void);
void ed_new(void);
void ed_save(void);
void ed_save_as(void);
int ed_save_changes(void);


HINSTANCE hInst;
HWND	  hMainWnd;
HMENU	  hMenu;
HWND	  hSplitter;
HWND      hButtonBar;
HWND      hStatus;
HWND	  hEdit;
HWND	  hError=NULL;

#define S_WIDTH 200

void ide_status(char *message)
{ //PostMessage(hStatus, WM_SETTEXT,0,(LPARAM)message);
  //UpdateWindow(hStatus);
	SetWindowText(hStatus,message);
}



#define ERROR_MARKER 0

void ide_add_error(char *message, int line_no)
{ int item;
  if (hError==NULL) return;
  item = (int) SendMessage(hError,LB_ADDSTRING,0,(LPARAM)message);
  if (item==LB_ERR||item==LB_ERRSPACE) return;
  SendMessage(hError,LB_SETITEMDATA,(WPARAM)item,(LPARAM)(line_no));
}

static int previous_error_line_no=-1;

void ide_clear_error(void)
{   if (hError==NULL) return;
	SendMessage(hError,LB_RESETCONTENT ,0,0);
    ed_send(SCI_MARKERDELETEALL,ERROR_MARKER,0);
	previous_error_line_no=-1;
}

void ide_mark_error(int line_no)
{ 
  if (previous_error_line_no>=0)
	ed_send(SCI_MARKERDELETE, previous_error_line_no-1, ERROR_MARKER);
  if (line_no>=0) {
    ed_send(SCI_MARKERADD, line_no-1, ERROR_MARKER);
    ed_send(SCI_GOTOLINE,line_no-1,0); /* first line is 0 */
  }
  previous_error_line_no=line_no;
}

#define MMIX_LINE_MARGIN 0
#define MMIX_BREAKX_MARKER 1
#define MMIX_BREAK_MARGIN 1
#define MMIX_TRACE_MARKER 2
#define MMIX_TRACE_MARGIN 2

static int previous_mmix_line_no = -1;

void stopped_at_line(int line_no)
/* called from MMIX Thread */
{ 
  if (previous_mmix_line_no > 0)
	PostMessage(hEdit,SCI_MARKERDELETE,previous_mmix_line_no-1,MMIX_TRACE_MARKER);
  if (line_no>0) {
    PostMessage(hEdit,SCI_MARKERADD, line_no-1, MMIX_TRACE_MARKER);
	PostMessage(hEdit,SCI_GOTOLINE,line_no-1,0); /* first line is 0 */
  }
  previous_mmix_line_no=line_no;
  memory_update();
}

void ide_clear_mmix_line(void)
{  ed_send(SCI_MARKERDELETEALL,MMIX_TRACE_MARKER,0);
   previous_mmix_line_no=-1;
}

int ide_connect(void)
{ if (vmb.connected) return 1;	
  if (DialogBox(hInst,MAKEINTRESOURCE(IDD_CONNECT),hMainWnd,ConnectDialogProc))
     init_mmix_bus(host,port,"MMIX IDE");
  return vmb.connected;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 	
  switch (message) 
  {  
  case WM_CREATE: 
		return 0;
  case WM_SIZE:
		if (wParam==SIZE_RESTORED || SIZE_MAXIMIZED)
		{	MoveWindow(hButtonBar, 0,0,LOWORD(lParam)-S_WIDTH,BB_HEIGHT, TRUE);
		    MoveWindow(hStatus, LOWORD(lParam)-S_WIDTH,0,S_WIDTH,BB_HEIGHT, TRUE);
			MoveWindow(hSplitter, 0,BB_HEIGHT,LOWORD(lParam),HIWORD(lParam)-BB_HEIGHT, TRUE);
		}
		return 1;
  case WM_PARENTNOTIFY:
		if (LOWORD(wParam)==WM_DESTROY)
		{ HWND hChildWnd = (HWND)lParam;
		  if (hChildWnd==hError)
			  hError=NULL;
		  else if (hChildWnd==hEdit)
		  {   ed_save_changes();		  
		      hEdit=NULL;
		  }
		}
		return 1;
  case WM_NOTIFY:
	  { NMHDR *n = (NMHDR*)lParam;
	    if (n->code == SCN_MARGINCLICK)
		{ struct SCNotification *sn=(struct SCNotification*)n;
		  int line, brkp;
		  line = (int)ed_send(SCI_LINEFROMPOSITION,sn->position,0);
		  brkp = (int)ed_send(SCI_MARKERGET,line,0);
		  brkp &= 1<<MMIX_BREAKX_MARKER;
          if (brkp)
		  { del_breakpoint(0,line+1);
		    ed_send(SCI_MARKERDELETE,line,MMIX_BREAKX_MARKER);
		  }
		  else
		  { 
		    if (set_breakpoint(0,line+1))
		      ed_send(SCI_MARKERADD,line,MMIX_BREAKX_MARKER);
		  }
		}
	  }
	  return 0;

  case WM_COMMAND:
    if (lParam==0)
	{ /* if (HIWORD(wParam)==0)  Menu */
      switch(LOWORD(wParam))
	  { case ID_FILE_EXIT:
		  if (!ed_save_changes()) return 0;
		  DestroyWindow(hWnd);
	      return 0;
	    case ID_FILE_NEW:
		  ed_new();
		  return 0;
	    case ID_FILE_OPEN:
		  ed_open();
		  return 0;
	    case ID_FILE_CLOSE:
		  if (ed_save_changes()) 
			  ed_send(WM_CLOSE,0,0);
	      return 0;
	    case ID_FILE_SAVE:
		  ed_save();
		  return 0;
	    case ID_FILE_SAVEAS:
		  ed_save_as();
		  return 0;
	    case ID_EDIT_UNDO:
		  ed_send(SCI_UNDO,0,0);
		  return 0;
	    case ID_EDIT_REDO:
		  ed_send(SCI_REDO,0,0);
		  return 0;
	    case ID_EDIT_CUT:
		  ed_send(SCI_CUT,0,0);
		  return 0;
	    case ID_EDIT_COPY:
		  ed_send(SCI_COPY,0,0);
		  return 0;
	    case ID_EDIT_PASTE:
		  ed_send(SCI_PASTE,0,0);
		  return 0;
	    case ID_EDIT_DELETE:
		  ed_send(SCI_CLEAR,0,0);
		  return 0;
	    case ID_EDIT_SELECTALL:
		  ed_send(SCI_SELECTALL,0,0);
		  return 0;
	    case ID_VIEW_ZOOMIN:
		  ed_send(SCI_ZOOMIN,0,0);
		      set_pos_key(hMainWnd,defined);
		  return 0;
	    case ID_VIEW_ZOOMOUT:
		  ed_send(SCI_ZOOMOUT,0,0);
		  return 0;
		case ID_VIEW_WHITESPACE:
          if (GetMenuState(hMenu,ID_VIEW_WHITESPACE,MF_BYCOMMAND)&MF_CHECKED)
		  {  ed_send(SCI_SETVIEWWS,SCWS_INVISIBLE,0);
		     ed_send(SCI_SETVIEWEOL,0,0);
			 CheckMenuItem(hMenu,ID_VIEW_WHITESPACE,MF_BYCOMMAND|MF_UNCHECKED);
		  }
		  else
		  {  ed_send(SCI_SETVIEWWS,SCWS_VISIBLEALWAYS,0);
		     ed_send(SCI_SETVIEWEOL,1,0);
			 CheckMenuItem(hMenu,ID_VIEW_WHITESPACE,MF_BYCOMMAND|MF_CHECKED);
		  }
		  return 0;
	    case ID_MMIX_RUN:
		  if (!ed_save_changes()) return 0;
		  if (!assemble_if_needed()) return 0;
		  if (!ide_connect()) return 0;
		  mmix_run();
	      return 0; 
		case ID_MMIX_DEBUG:
		  if (!ed_save_changes()) return 0;
		  if (!assemble_if_needed()) return 0;
		  if (!ide_connect()) return 0;
		  update_breakpoints();
		  set_debug_windows();
		  mmix_debug();
	      return 0; 
		case ID_MMIX_STOP:
		  mmix_stop();
		  return 0;
	    case ID_MMIX_ASSEMBLE:
		  ed_save_changes();
		  mmix_assemble(0);
	      return 0; 

	    case ID_MMIX_CONNECT:
	      if (!vmb.connected)
			  ide_connect();
	      else
	        vmb_disconnect(&vmb);
	      return 0;

	    case ID_MEM_TEXTSEGMENT:
		  if (!ide_connect()) return 0;
		  new_memory_view(0);
		  return 0;
	    case ID_MEM_DATASEGMENT:
		  if (!ide_connect()) return 0;
		  new_memory_view(1);
		  return 0;
	    case ID_MEM_POOLSEGMENT:
		  if (!ide_connect()) return 0;
		  new_memory_view(2);
		  return 0;
	    case ID_MEM_STACKSEGMENT:
		  if (!ide_connect()) return 0;
		  new_memory_view(3);
		  return 0;
	    case ID_MEM_NEGATIVESEGMENT:
		  if (!ide_connect()) return 0;
		  new_memory_view(4);
		  return 0;
	    case ID_REGISTERS_LOCAL:
		  new_register_view(0);
		  return 0;
	    case ID_REGISTERS_GLOBAL:
		  new_register_view(1);
		  return 0;
		case ID_REGISTERS_STACK:
		  new_register_view(1);
		  return 0;
	    case ID_REGISTERS_SPECIAL:
		  new_register_view(3);
		  return 0;
		case ID_OPTIONS_SPECIALS:
		  DialogBox(hInst,MAKEINTRESOURCE(IDD_SHOW_SPECIAL),hWnd,OptionSpecialDialogProc);
		  return 0;
		case ID_OPTIONS_WINDOWS:
		  DialogBox(hInst,MAKEINTRESOURCE(IDD_SHOW_DEBUG),hWnd,OptionDebugDialogProc);
		  return 0;
	    case ID_HELP_ABOUT:
	      DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,AboutDialogProc);
	      return 0; 
	  }
	  /* else if (HIWORD(wParam)==1) break; Accellerator */
	}
	else /* Control-defined notification code */
	{ HWND hControl = (HWND)lParam;
	  if (hControl==hError)
	  { int item, line_no;
		item = (int)SendMessage(hError,LB_GETCURSEL,0,0);
	    if (item!=LB_ERR)
		  line_no = (int)SendMessage(hError,LB_GETITEMDATA,item,0);
		else
		  line_no = -1;
		switch (HIWORD(wParam))
		{ case LBN_DBLCLK:
//			ide_mark_error(line_no);
			SetFocus(hEdit);
			return 0;
		  case LBN_SELCHANGE:
			ide_mark_error(line_no);
			return 0;
		  case   LBN_SELCANCEL:
  			ide_mark_error(-1);
			return 0;
		  case LBN_SETFOCUS:
			  break;
		   case LBN_KILLFOCUS:
			  break;
		  default:
			  return 0;
		}
	  }
	}
    return 0;
  case WM_MMIX_STOPPED:
    stopped_at_line((int)wParam);
	return 0;
  case WM_CLOSE:
    ed_save_changes();
	DestroyWindow(hWnd);
	return 0;
  case WM_DESTROY:
	set_pos_key(hMainWnd,defined);
    PostQuitMessage(0);
    return 0;
  case WM_SYSCOMMAND:
	if (wParam==SC_CLOSE)
	{ if (!ed_save_changes()) 
		return 0;
	}
	break;
  }
 return (DefWindowProc(hWnd, message, wParam, lParam));
}

static char classname[]="MMIXIDECLASS";

BOOL InitInstance(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;




  ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_MENUBAR+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MENU);
	wcex.lpszClassName = classname;
	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) return FALSE;


    hMainWnd = CreateWindow(classname, title ,
				WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,
	            NULL, NULL, hInstance, NULL);

   hMenu = GetMenu(hMainWnd);

   return TRUE;
}

/* the editor window */


SciFnDirect ed_fn;
sptr_t ed_ptr;
char fullname[MAX_PATH+1]={0};
#define ED_BLOCKSIZE 0x800
void init_edit(HINSTANCE hInstance)
{	Scintilla_RegisterClasses(hInstance);
	Scintilla_LinkLexers();
}


sptr_t ed_send(unsigned int msg,uptr_t wparam,sptr_t lparam)
{  if (hEdit==NULL) return 0;
	return ed_fn(ed_ptr,msg,wparam,lparam);
  // Scintilla_DirectFunction
}


void new_edit(void)
{  int stylebits;
   sp_create_options(1,0,0.8,0,NULL);
   hEdit = CreateWindowEx(0,"Scintilla","Editor",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN,
		0,0,400,300,hSplitter,NULL,hInst,NULL);
   ed_fn = (SciFnDirect)SendMessage(hEdit,SCI_GETDIRECTFUNCTION,0,0);
   ed_ptr= (sptr_t)SendMessage(hEdit,SCI_GETDIRECTPOINTER,0,0);

  /* configure the style */
   ed_send(SCI_STYLESETFONT,STYLE_DEFAULT,(sptr_t)"Courier New");
   ed_send(SCI_STYLESETSIZE,STYLE_DEFAULT,(sptr_t)12);
   ed_send(SCI_STYLESETBOLD,STYLE_DEFAULT,(sptr_t)1);
   /* configure the MMIXAL lexer */
   ed_send(SCI_SETLEXER,SCLEX_MMIXAL,0);
   stylebits= (int)ed_send(SCI_GETSTYLEBITSNEEDED,0,0);
   ed_send(SCI_SETSTYLEBITS,stylebits,0);
   
   ed_send(SCI_SETKEYWORDS,0,(sptr_t)mmix_opcodes);
   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_OPCODE_VALID,RGB(0,0,0xA0));
   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_OPCODE_UNKNOWN,RGB(0xFF,0,0));

   ed_send(SCI_SETKEYWORDS,1,(sptr_t)mmix_special_registers);
   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_REGISTER,RGB(0,0,0xFF));

   ed_send(SCI_SETKEYWORDS,2,(sptr_t)mmix_predefined);
   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_SYMBOL,RGB(0,0x80,0xFF));

   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_COMMENT,RGB(0,0xA0,0));

   /* configure margins and markers */
   /* line numbers */
   ed_send(SCI_SETMARGINTYPEN,MMIX_LINE_MARGIN,SC_MARGIN_NUMBER);
   ed_send(SCI_SETMARGINWIDTHN,MMIX_LINE_MARGIN,40);
   /* errors */
   ed_send(SCI_MARKERDEFINE, ERROR_MARKER,SC_MARK_BACKGROUND);
   ed_send(SCI_MARKERSETBACK, ERROR_MARKER,RGB(0xFF,0xE0,0xE0));
   ed_send(SCI_MARKERSETFORE, ERROR_MARKER,RGB(0,0,0));

   /* break points */
   ed_send(SCI_SETMARGINTYPEN,MMIX_TRACE_MARGIN,SC_MARGIN_SYMBOL);
   ed_send(SCI_SETMARGINWIDTHN,MMIX_TRACE_MARGIN,16);
   ed_send(SCI_SETMARGINMASKN,MMIX_TRACE_MARGIN,(1<<MMIX_TRACE_MARKER));
   ed_send(SCI_MARKERDEFINE, MMIX_BREAKX_MARKER,SC_MARK_CIRCLE);
   ed_send(SCI_MARKERSETBACK, MMIX_BREAKX_MARKER,RGB(0x00,0x00,0xFF));
   ed_send(SCI_MARKERSETFORE, MMIX_BREAKX_MARKER,RGB(0x00,0x00,0xFF));
   /* tracing */
   ed_send(SCI_SETMARGINTYPEN,MMIX_BREAK_MARGIN,SC_MARGIN_SYMBOL);
   ed_send(SCI_SETMARGINWIDTHN,MMIX_BREAK_MARGIN,16);
   ed_send(SCI_SETMARGINSENSITIVEN,MMIX_BREAK_MARGIN,1);
   ed_send(SCI_SETMARGINMASKN,MMIX_BREAK_MARGIN,(1<<MMIX_BREAKX_MARKER));
   ed_send(SCI_MARKERDEFINE, MMIX_TRACE_MARKER,SC_MARK_ARROW);
   ed_send(SCI_MARKERSETBACK, MMIX_TRACE_MARKER,RGB(0xFF,0x00,0x00));
   ed_send(SCI_MARKERSETFORE, MMIX_TRACE_MARKER,RGB(0xFF,0,0));


   /* kommand keys for the edit windows*/
   ed_send(SCI_ASSIGNCMDKEY,VK_OEM_PLUS+(SCMOD_CTRL<<16),SCI_ZOOMIN);
   ed_send(SCI_ASSIGNCMDKEY,VK_OEM_MINUS+(SCMOD_CTRL<<16),SCI_ZOOMOUT);
}

void update_breakpoints(void)
{ int line = -1;
  while ((line = (int)ed_send(SCI_MARKERNEXT,line+1,(1<<MMIX_BREAKX_MARKER)))>=0)
   if (!set_breakpoint(0,line+1))
     ed_send(SCI_MARKERDELETE,line,MMIX_BREAKX_MARKER);
}

void ed_new(void)
{   if (hEdit==NULL) new_edit();
	if (!ed_save_changes()) return;
	ed_send(SCI_CLEARALL,0,0);
	ed_send(SCI_EMPTYUNDOBUFFER,0,0);
	fullname[0] = '\0';
	SetWindowText(hMainWnd,fullname);
	ed_send(SCI_SETSAVEPOINT,0,0);
	ed_send(SCI_CANCEL,0,0);
}

void ed_open_file(char *name)
{ int n;
  char *shortname;
  FILE *fp;
  ed_new();
  ed_send(SCI_SETUNDOCOLLECTION,0,0);
  n = GetFullPathName(name,MAX_PATH,fullname,&shortname);
  if (n>MAX_PATH)
  { ide_status("Filename too long");
    return;
  }
  else if (n<=0)
  { ide_status("Filename not legal");
    return;
  }
  fp = fopen(fullname, "rb");
  if (fp) {
    char data[ED_BLOCKSIZE];
    size_t len;
    SetWindowText(hMainWnd,shortname);
    while ((len = fread(data, 1, ED_BLOCKSIZE, fp))>0)
      ed_send(SCI_ADDTEXT, len, (sptr_t)data);
	fclose(fp); 
    ed_send(SCI_SETUNDOCOLLECTION, 1,0);
    ed_send(SCI_SETSAVEPOINT,0,0);
    ed_send(SCI_GOTOPOS,0,0);
  }
  else
	  ide_status("Unable to open file");
}

void ed_open(void)
{ 	char name[MAX_PATH+1] = "\0";
	OPENFILENAME ofn={0};
	ofn.lStructSize= sizeof(OPENFILENAME);
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFile = name;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "MMIX (.mms)\0*.mms\0" "All Files (*.*)\0*.*\0\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrTitle = "Open File";
	ofn.Flags = OFN_HIDEREADONLY;
    if (GetOpenFileName(&ofn)) 
    ed_open_file(name);
}




int dont_overwrite(char *fullname)
{ HANDLE out_file = CreateFile(fullname,GENERIC_READ,FILE_SHARE_READ,
                       (LPSECURITY_ATTRIBUTES) NULL,OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,NULL);
  if (out_file != INVALID_HANDLE_VALUE) 
  { LARGE_INTEGER size;
    GetFileSizeEx(out_file,&size);
	CloseHandle(out_file);
	if (size.QuadPart!=0 
	      && IDYES!=MessageBox(NULL,"Overwrite existing file?",
	                       fullname,MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2))
      return 1;
  }
  return 0;
}


void ed_save_name(char *fullname)
{	FILE *fp;
    if (fullname[0]==0) return;
    fp = fopen(fullname, "wb");
	if (fp) {
		int i;
		struct Sci_TextRange tr;
		char data[ED_BLOCKSIZE+1];
		int len = (int)ed_send(SCI_GETLENGTH,0,0);
	    tr.lpstrText = data;
		for (i = 0; i < len; i += ED_BLOCKSIZE) 
		{ int next = len - i;
		  if (next > ED_BLOCKSIZE) next = ED_BLOCKSIZE;
	      tr.chrg.cpMin = i;
	      tr.chrg.cpMax = i+next;
	      ed_send(SCI_GETTEXTRANGE, 0,(sptr_t)&tr);
		  fwrite(data, next, 1, fp);
		}
		fclose(fp);
		ed_send(SCI_SETSAVEPOINT,0,0);
	}
}

int mmo_file_newer(void)
{ HANDLE mms, mmo;
  FILETIME mmsTime, mmoTime;
  char *mmo_name;
  if (fullname[0]==0) return 0;
  mms = CreateFile(fullname,FILE_READ_ATTRIBUTES,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (mms==INVALID_HANDLE_VALUE) return 0;
  mmo_name = get_mmo_name();
  if (mmo_name==NULL) return 0;
  mmo = CreateFile(mmo_name,FILE_READ_ATTRIBUTES,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (mms==INVALID_HANDLE_VALUE) return 1;
  GetFileTime(mms,NULL,NULL,&mmsTime);
  GetFileTime(mmo,NULL,NULL,&mmoTime);
  if (CompareFileTime(&mmsTime,&mmoTime)>0)
	  return 1;
  else
	  return 0;
}



void ed_save(void)
{ if (hEdit==NULL) return;
  if (fullname[0]==0)
    ed_save_as();
  else
	ed_save_name(fullname);
}

void ed_save_as(void)
{	char asname[MAX_PATH+1] = "\0";
	OPENFILENAME ofn ={0};
	if (hEdit==NULL) return;
	strcpy(asname, fullname);
	ofn.lStructSize= sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFile = asname;
	ofn.lpstrFilter = "MMIX (.mms)\0*.mms\0" "All Files (*.*)\0*.*\0\0";
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Save File";
	ofn.Flags = OFN_HIDEREADONLY;

	if (GetSaveFileName(&ofn) && !dont_overwrite(asname)) 
	{  	strncpy(fullname, asname,MAX_PATH);
		SetWindowText(hMainWnd,asname);
		ed_save_name(asname);
		//InvalidateRect(hEdit, NULL, NULL);
	}
}

int ed_save_changes(void)
{ int dirty;
  if (hEdit==NULL) return 1;
  dirty = (int)ed_send(SCI_GETMODIFY,0,0);
  if (dirty)
  {	 int decision = MessageBox(hMainWnd, "Save changes ?", "MMIX IDE", MB_YESNOCANCEL);
	 if (decision == IDYES)
	   ed_save();
	 else if (decision == IDCANCEL)
	   return 0;
  }
  return 1;
}


int assemble_if_needed(void)
{ if (mmo_file_newer())
  {	 int decision = MessageBox(hMainWnd, "mms file newer than mmo file. Assemble?", "MMIX IDE", MB_YESNOCANCEL);
	 if (decision == IDYES)
	   mmix_assemble(0);
	 else if (decision == IDCANCEL)
	   return 0;
  }
  if (!has_debug_info())
  {	 int decision = MessageBox(hMainWnd, "no debug information available. Assemble?", "MMIX IDE", MB_YESNOCANCEL);
	 if (decision == IDYES)
	   mmix_assemble(0);
	 else if (decision == IDCANCEL)
	   return 0;
  }
  return 1;
}

void new_errorlist(void)
{   sp_create_options(0,0,0.2,0,hEdit);
	hError = CreateWindow("LISTBOX", "Errorlist" ,
				WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT 
,
                0,0,0,0,
	            hSplitter, NULL, hInst, NULL);
}

#define BG_FILE 0
#define BG_EDIT 1
#define BG_VIEW 2
#define BG_MMIX 3
#define BG_HELP 4

void add_button(int iconID, int menuID, int group, int buttonID)
{ HANDLE h;
  h = LoadImage(hInst, MAKEINTRESOURCE(iconID),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADTRANSPARENT);
  bb_CreateButton(hButtonBar,h,menuID,group,buttonID,1,1);
}

void add_buttons(void)
{ 
  add_button(IDI_FILE_NEW,ID_FILE_NEW,BG_FILE,0);
  add_button(IDI_FILE_OPEN,ID_FILE_OPEN,BG_FILE,1);
  add_button(IDI_FILE_SAVE,ID_FILE_SAVE,BG_FILE,2);
  add_button(IDI_EDIT_COPY,ID_EDIT_COPY,BG_EDIT,3);
  add_button(IDI_EDIT_PASTE,ID_EDIT_PASTE,BG_EDIT,4);
  add_button(IDI_EDIT_CUT,ID_EDIT_CUT,BG_EDIT,5);
  add_button(IDI_EDIT_UNDO,ID_EDIT_UNDO,BG_EDIT,6);
  add_button(IDI_EDIT_REDO,ID_EDIT_REDO,BG_EDIT,7);
  add_button(IDI_VIEW_ZOOMIN,ID_VIEW_ZOOMIN,BG_VIEW,8);
  add_button(IDI_VIEW_ZOOMOUT,ID_VIEW_ZOOMOUT,BG_VIEW,9);
  add_button(IDI_MMIX_DEBUG,ID_MMIX_DEBUG,BG_MMIX,10);
  add_button(IDI_HELP,ID_HELP_ABOUT,BG_MMIX,10);
}






int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG msg;
	HACCEL hAccelTable;
	RECT r;

	hInst = hInstance; 
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	sp_init(hInstance);
    bb_init(hInstance);

	if (!InitInstance (hInstance)) return FALSE;

    GetClientRect(hMainWnd,&r);
    hButtonBar = bb_CreateButtonBar("The Button Bar", WS_CHILD|WS_VISIBLE,
		0, 0, r.right-r.left-S_WIDTH, BB_HEIGHT , hMainWnd, NULL, hInst, NULL);
	hSplitter = sp_CreateSplitter("The Splitter", WS_CHILD|WS_VISIBLE,
                  0, BB_HEIGHT, r.right-r.left, r.bottom-r.top-BB_HEIGHT, hMainWnd, NULL, hInst, NULL);
    add_buttons();

	init_edit(hInstance);
	mmix_lib_init();
	new_edit();
	hStatus = CreateWindow("STATIC", "Status" ,
				WS_CHILD|WS_VISIBLE|SS_CENTER,
                r.right-r.left-S_WIDTH,0, S_WIDTH,BB_HEIGHT,
	            hMainWnd, NULL, hInstance, NULL);

	vmb_program_name="mmixide";
    vmb_message_hook = win32_message;
	vmb_debug_hook = NULL;
	vmb_error_init_hook = NULL;
    vmb_exit_hook=ide_exit_ignore;
    programhelpfile="mmixide.hlp";
	/* normal vmb processing */
	  param_init ();
      get_pos_key(&xpos,&ypos,defined);
      SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);




    ShowWindow(hMainWnd, SW_SHOW);
	if (minimized)CloseWindow(hMainWnd); 
    
	UpdateWindow(hMainWnd);




	while (GetMessage(&msg, NULL, 0, 0)) 
	  if (!TranslateAccelerator(hMainWnd, hAccelTable, &msg) &&
		  !do_subwindow_msg(&msg) ) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	Scintilla_ReleaseResources();
	vmb_end();
	return (int)msg.wParam;
}

void  ide_exit_ignore(int returncode)
{
;
}