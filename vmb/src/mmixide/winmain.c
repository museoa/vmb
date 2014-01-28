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
#include "findreplace.h"
#include "print.h"
#include "info.h"
#include "filelist.h"
#include "symtab.h"
#include "mmixdata.h"
#include "edit.h"
#include "assembler.h"
#include "sources.h"
#include "debug.h"
#include "breakpoints.h"
#include "winmain.h"

#define STATIC_BUILD
#include "../scintilla/include/scintilla.h"
#include "../scintilla/include/scilexer.h"
int major_version=1, minor_version=0;
char version[]="$Revision: 1.17 $ $Date: 2014-01-28 11:46:40 $";
char title[] ="VMB MMIX IDE";

/* Button groups for the button bar */
#define BG_FILE 0
#define BG_EDIT 1
#define BG_VIEW 2
#define BG_MMIX 3
#define BG_DEBUG 4
#define BG_HELP 5

sptr_t ed_send(unsigned int msg,uptr_t wparam,sptr_t lparam);
int ed_open(void);
int ed_close(void);
int ed_close_all(int cancel);
void ed_new(void);
void ed_save(void);
void ed_save_as(void);
int ed_save_changes(int cancel);
void set_lineno_width(void);

HINSTANCE hInst;
HWND	  hMainWnd;
HMENU	  hMenu;
HWND	  hSplitter;
HWND      hButtonBar;
HWND      hStatus;
HWND	  hEdit;
HWND	  hError=NULL;
HWND	  hFileList=NULL;
HWND	  hSymbolTable=NULL;


#define S_WIDTH 200

void ide_status(char *message)
{ SendMessage(hStatus, WM_SETTEXT,0,(LPARAM)message);
  UpdateWindow(hStatus);
  //SetWindowText(hStatus,message);
}



#define ERROR_MARKER 0


void ide_add_error(char *message, int file_no, int line_no)
{ int item;
  if (hError==NULL) new_errorlist();
  item = (int) SendMessage(hError,LB_ADDSTRING,0,(LPARAM)message);
  if (item==LB_ERR||item==LB_ERRSPACE) return;
  SendMessage(hError,LB_SETITEMDATA,(WPARAM)item,item_data(file_no, line_no));
}

static int previous_error_line_no=-1;

void ide_clear_error_marker(void)
{ ed_send(SCI_MARKERDELETEALL,ERROR_MARKER,0);
  previous_error_line_no=-1;
}

void ide_clear_error_list(void)
/* clear error markers in current edit file */
{   if (hError==NULL) return;
	SendMessage(hError,LB_RESETCONTENT ,0,0);
	ide_clear_error_marker();
}

void ide_mark_error(int file_no, int line_no)
{ 
  set_edit_file(file_no);
  if (previous_error_line_no>=0)
	ed_send(SCI_MARKERDELETE, previous_error_line_no-1, ERROR_MARKER);
  if (line_no>=0) {
    ed_send(SCI_MARKERADD, line_no-1, ERROR_MARKER);
    ed_send(SCI_GOTOLINE,line_no-1,0); /* first line is 0 */
  }
  previous_error_line_no=line_no;
}


static int previous_mmix_line_no = -1;

void clear_stop_marker(void)
{ if (previous_mmix_line_no > 0)
  {  SendMessage(hEdit,SCI_MARKERDELETEALL,MMIX_TRACE_MARKER,0);  
     previous_mmix_line_no= -1;
  }
}

void show_stop_marker(int file_no, int line_no)
{ set_edit_file(file_no);
  if (previous_mmix_line_no > 0)
	PostMessage(hEdit,SCI_MARKERDELETE,previous_mmix_line_no-1,MMIX_TRACE_MARKER);
  if (line_no>0) {
    PostMessage(hEdit,SCI_MARKERADD, line_no-1, MMIX_TRACE_MARKER);
	PostMessage(hEdit,SCI_ENSUREVISIBLEENFORCEPOLICY,line_no-1,0); 
//	PostMessage(hEdit,SCI_GOTOLINE,line_no-1,0); /* first line is 0 */
  }
  previous_mmix_line_no=line_no;
}

void ide_clear_mmix_line(void)
{  ed_send(SCI_MARKERDELETEALL,MMIX_TRACE_MARKER,0);
   previous_mmix_line_no=-1;
}

extern int auto_connect;

int ide_connect(void)
{ if (vmb.connected) return 1;
  if (auto_connect)
	init_mmix_bus(host,port,"MMIX IDE");
  else
  { int decision = MessageBox(hMainWnd, "Connect to Motherboard ?","VMB Connect",MB_YESNOCANCEL);
	if (decision == IDYES)
       init_mmix_bus(host,port,"MMIX IDE");
  }
  return vmb.connected;
}

static int menu_toggle(int id)
/* toggle the menu id and return its new status */
{ if (GetMenuState(hMenu,id,MF_BYCOMMAND)&MF_CHECKED)
  { CheckMenuItem(hMenu,id,MF_BYCOMMAND|MF_UNCHECKED);
    return 0;
 }
  else
  { CheckMenuItem(hMenu,id,MF_BYCOMMAND|MF_CHECKED);
    return 1;
  }
}

int fontsize;
int show_whitespace=0;

void set_whitespace(void)
{  if (show_whitespace)
		  {  ed_send(SCI_SETVIEWWS,SCWS_VISIBLEALWAYS,0);
		     ed_send(SCI_SETVIEWEOL,1,0);
		  }
  		  else
		  {  ed_send(SCI_SETVIEWWS,SCWS_INVISIBLE,0);
		     ed_send(SCI_SETVIEWEOL,0,0);
		  }
}

void mms_style(void);
void txt_style(void);
int syntax_highlighting = 0;



void set_text_style(void)
{ if (syntax_highlighting)
     mms_style();
  else
     txt_style();
}

int codepage=0;

void set_edit_file(int file_no)
{ if (hEdit==NULL) new_edit();
  if (file_no==edit_file_no || file_no<0) return;
  ide_clear_error_marker();
  clear_stop_marker();
  if (edit_file_no>=0)
    file2dirty(edit_file_no)=(int)ed_send(SCI_GETMODIFY,0,0);
  edit_file_no = file_no;
  set_edit_document();
  set_tabwidth();
  set_text_style();
  set_whitespace();
  ed_send(SCI_SETCODEPAGE,codepage,0);
  SetWindowText(hMainWnd,file_listname(file_no));
  file_list_mark(edit_file_no);
  update_symtab();
}

ide_mark_breakpoint(int file_no, int line_no)
{ if (file_no==edit_file_no)
     ed_send(SCI_MARKERADD,line_no-1,MMIX_BREAKX_MARKER); /* lines start with zero */
}

static int all_saved=1;
static int all_cancel=0;
static int current_file_no=-1;

void save_file_if_needed(int file_no)
{ if (!all_saved) return;
  if (file_no==current_file_no) return;
  if (!file2dirty(file_no)) return;
  set_edit_file(file_no);
  if (!ed_save_changes(all_cancel)) all_saved=0;
}

int ed_save_all(int cancel)
{ current_file_no = edit_file_no;
  all_saved=1;
  all_cancel=cancel;
  for_all_files(save_file_if_needed);
  set_edit_file(current_file_no);
  return all_saved && ed_save_changes(cancel);
}


int ide_prepare_mmix(void)
{ if (!ed_save_all(1)) return 0;
  if (!assemble_all_needed()) return 0;
  if (!ide_connect()) return 0;
  if (application_file_no==edit_file_no)
  { MessageBox(hMainWnd,"Application is already running.\r\nReset to load new code.","MMIX IDE",MB_OK);
    return 0;
  }
  else if (application_file_no>=0)
  { MessageBox(hMainWnd,"Different Application is already running.","MMIX IDE",MB_OK);
    return 0;
  }
  bb_set_group(hButtonBar,BG_DEBUG,1,1);
  return 1;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 	
  switch (message) 
  {     case WM_CREATE:
         SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
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
		  {
			hError=NULL;
		  }
		  else if (hChildWnd==hEdit)
		  {   ed_close_all(0);		  
		      hEdit=NULL;
		  }
		  else if (hChildWnd==hFileList)
		  { CheckMenuItem(hMenu,ID_VIEW_FILELIST,MF_BYCOMMAND|MF_UNCHECKED);
		    hFileList=NULL;
		  }
		  else if (hChildWnd==hSymbolTable)
		  { CheckMenuItem(hMenu,ID_VIEW_SYMBOLTABLE,MF_BYCOMMAND|MF_UNCHECKED);
		    hSymbolTable=NULL;
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
		  { del_breakpoint(edit_file_no,line+1);
		    ed_send(SCI_MARKERDELETE,line,MMIX_BREAKX_MARKER);
		  }
		  else
		  { 
		    if (set_breakpoint(edit_file_no,line+1))
		      ed_send(SCI_MARKERADD,line,MMIX_BREAKX_MARKER);
		  }
		}
		else if(n->code==SCN_MODIFIED)
		{ struct SCNotification *sn=(struct SCNotification*)n;
		  if (sn->linesAdded!=0) clear_stop_marker();
		}
	  }
	  return 0;
  case WM_MEASUREITEM:
    if (wParam==GetWindowLong(hFileList,GWL_ID))
		return file_list_measureitem((LPMEASUREITEMSTRUCT)lParam);
	else if (wParam==GetWindowLong(hSymbolTable,GWL_ID))
		return symtab_measureitem((LPMEASUREITEMSTRUCT)lParam);
	else
	    break;
  case WM_DRAWITEM: 
     if (wParam==GetWindowLong(hFileList,GWL_ID))
		return file_list_drawitem((LPDRAWITEMSTRUCT)lParam);
	else if (wParam==GetWindowLong(hSymbolTable,GWL_ID))
		return symtab_drawitem((LPDRAWITEMSTRUCT)lParam);
	else
	    break;

  case WM_COMMAND:
    if (lParam==0)
	{ /* if (HIWORD(wParam)==0)  Menu */
      switch(LOWORD(wParam))
	  { case ID_FILE_EXIT:
	      if (ed_close_all(1))
		    DestroyWindow(hWnd);
	      return 0;
	    case ID_FILE_NEW:
		  ed_new();
		  return 0;
	    case ID_FILE_OPEN:
		  set_edit_file(ed_open());
		  return 0;
	    case ID_FILE_CLOSE:
          ed_close();
	      return 0;
	    case ID_FILE_SAVE:
		  ed_save();
		  return 0;
	    case ID_FILE_SAVEAS:
		  ed_save_as();
		  return 0;
	    case ID_FILE_PAGESETUP:
		  page_setup();
		  return 0;
	    case ID_FILE_PRINT:
		  print();
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
		case ID_EDIT_FIND:
			{ HWND h;
			  h = CreateDialog(hInst,MAKEINTRESOURCE(IDD_FIND),hWnd,FindDialogProc);
			  register_subwindow(h);
			  ShowWindow(h, SW_SHOW); 
			}
			return 0;
		case ID_EDIT_FINDAGAIN:
			find_again();
			return 0;
		case ID_EDIT_REPLACE:
			replace_again();
			return 0;
		case ID_VIEW_FILELIST:
		  if (menu_toggle(ID_VIEW_FILELIST))
		  { if (hFileList!=NULL) return 0;
			create_filelist();
		  }
		  else
		  {  DestroyWindow(hFileList);
			 hFileList=NULL;
		  }
		  return 0;
		case ID_VIEW_SYMBOLTABLE:
		  if (menu_toggle(ID_VIEW_SYMBOLTABLE))
		  { if (hSymbolTable!=NULL) return 0;
			create_symtab();
		  }
		  else
		  {  DestroyWindow(hSymbolTable);
			 hSymbolTable=NULL;
		  }
		  return 0;
	    case ID_VIEW_ZOOMIN:
		  ed_send(SCI_ZOOMIN,0,0);
		  set_lineno_width();
		  return 0;
	    case ID_VIEW_ZOOMOUT:
		  ed_send(SCI_ZOOMOUT,0,0);
		  set_lineno_width();
		  return 0;
		case ID_VIEW_WHITESPACE:
          show_whitespace=menu_toggle(ID_VIEW_WHITESPACE);
		  set_whitespace();
		  return 0;
		case ID_ENCODING_ASCII:
		  ed_send(SCI_SETCODEPAGE,0,0);
		  codepage=0;
          CheckMenuItem(hMenu,ID_ENCODING_ASCII,MF_BYCOMMAND|MF_CHECKED);
          CheckMenuItem(hMenu,ID_ENCODING_UTF,MF_BYCOMMAND|MF_UNCHECKED);
          return 0;
		case ID_ENCODING_UTF:
		  ed_send(SCI_SETCODEPAGE,SC_CP_UTF8,0);
		  codepage=SC_CP_UTF8;
          CheckMenuItem(hMenu,ID_ENCODING_ASCII,MF_BYCOMMAND|MF_UNCHECKED);
          CheckMenuItem(hMenu,ID_ENCODING_UTF,MF_BYCOMMAND|MF_CHECKED);
          return 0;
		case ID_VIEW_SYNTAX:
          syntax_highlighting=menu_toggle(ID_VIEW_SYNTAX);
		  set_text_style();
		  return 0;
		case ID_MMIX_STEP:
		  mmix_continue('n');
		  return 0;
	   case ID_MMIX_STOP:
		  mmix_stop();
		  return 0;
		case ID_MMIX_QUIT:
		  mmix_continue('q');
		  return 0;
		case ID_MMIX_DEBUG:
		  if (mmix_active())
		  { mmix_continue('c');
		  }
		  else 
		  { if (!ide_prepare_mmix()) return 0;
		    set_debug_windows();
		    mmix_debug(edit_file_no);
		  }
	      return 0; 
		case ID_MMIX_RUN:
		  if (mmix_active()) return 0;
		  if (!ide_prepare_mmix()) return 0;
		  mmix_run(edit_file_no);
	      return 0; 

	    case ID_MMIX_ASSEMBLE:
		  if (!ed_save_changes(1)) return 0;
		  if (mmix_assemble(edit_file_no)==0 && edit_file_no==application_file_no && vmb.power)
		  { MessageBox(hWnd,"Different mmo file running!", file_listname(edit_file_no),MB_OK|MB_ICONWARNING);
		  }
	      return 0; 
	    case ID_MMIX_CONNECT:
		  if (menu_toggle(ID_MMIX_CONNECT))
		  { if (!vmb.connected)
			ide_connect();
		  }
		  else
		  { vmb_disconnect(&vmb);
		  }
		  return 0;
		  if (vmb.connected) mmix_status(MMIX_CONNECTED);
		  else mmix_status(MMIX_DISCONNECTED);
		  CheckMenuItem(hMenu,ID_MMIX_CONNECT,MF_BYCOMMAND|(vmb.connected?MF_CHECKED:MF_UNCHECKED));
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
		  new_register_view(REG_LOCAL);
		  return 0;
	    case ID_REGISTERS_GLOBAL:
		  new_register_view(REG_GLOBAL);
		  return 0;
	    case ID_REGISTERS_SPECIAL:
		  new_register_view(REG_SPECIAL);
		  return 0;
		case ID_REGISTERS_STACK:
		  show_debug_regstack=1;
		  new_register_view(REG_LOCAL);
		  return 0;
		case ID_VIEW_BREAKPOINTS:
          create_breakpoints();
		  return 0;
		case ID_OPTIONS_EDITOR:
		  DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS_EDITOR),hWnd,OptionEditorDialogProc);
		  return 0;
		case ID_OPTIONS_SYMTAB:
		  DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS_SYMTAB),hWnd,OptionSymtabDialogProc);
		  return 0;
		case ID_OPTIONS_ASSEMBLER:
		  DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS_ASSEMBLER),hWnd,OptionAssemblerDialogProc);
		  return 0;
		case ID_OPTIONS_DEBUG:
		  DialogBox(hInst,MAKEINTRESOURCE(IDD_SHOW_DEBUG),hWnd,OptionDebugDialogProc);
		  return 0;
		case ID_OPTIONS_SOURCES:
		  DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS_SOURCES),hWnd,OptionSourcesDialogProc);
		  return 0;
		case ID_OPTIONS_VMB:
		  DialogBox(hInst,MAKEINTRESOURCE(IDD_CONNECT),hWnd,ConnectDialogProc);
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
	  { int item, line_no, file_no;
		item = (int)SendMessage(hError,LB_GETCURSEL,0,0);
	    if (item!=LB_ERR)
		{ LPARAM data;
		  data = SendMessage(hError,LB_GETITEMDATA,item,0);
		  line_no = item_line_no(data);
		  file_no = item_file_no(data);
		}
		else
		{ line_no = -1;
		  file_no = -1;
		}
		switch (HIWORD(wParam))
		{ case LBN_DBLCLK:
			SetFocus(hEdit);
			return 0;
		  case LBN_SELCHANGE:
			ide_mark_error(file_no,line_no);
			return 0;
		  case   LBN_SELCANCEL:
  			ide_mark_error(-1,-1);
			return 0;
		  case LBN_SETFOCUS:
			  break;
		   case LBN_KILLFOCUS:
			  break;
		  default:
			  return 0;
		}
	  }
	  else if (hControl==hFileList)
	  { int item, file_no;
		item = (int)SendMessage(hFileList,LB_GETCURSEL,0,0);
	    if (item!=LB_ERR)
		  file_no = (int)SendMessage(hFileList,LB_GETITEMDATA,item,0);
		else
		  file_no = -1;
		if (HIWORD(wParam)== LBN_DBLCLK)
		    set_edit_file(file_no);
	  }	  
	  else if (hControl==hSymbolTable && HIWORD(wParam)== LBN_DBLCLK)
	  { int item, line_no, file_no;
	    sym_node *sym=NULL;
		item = (int)SendMessage(hSymbolTable,LB_GETCURSEL,0,0);
	    if (item!=LB_ERR)
		  sym = (sym_node *)SendMessage(hSymbolTable,LB_GETITEMDATA,item,0);
		if (sym!=NULL)
		{ file_no=sym->file_no;
		  line_no=sym->line_no;
 		  set_edit_file(file_no);
		  ed_send(SCI_ENSUREVISIBLEENFORCEPOLICY,line_no-1,0); 
//		  ed_send(SCI_GOTOLINE,line_no-1,0); /* first line is 0 */
		  SetFocus(hEdit);
		}
	  }
	}
    return 0;
  case WM_MMIX_STOPPED:
    if (lParam==-1) /* terminated */   
	{ mmix_status(MMIX_HALTED);
      show_stop_marker(edit_file_no,-1); /* clear stop marker */
	  bb_set_group(hButtonBar,BG_DEBUG,0,0);
	  update_symtab();
	}
	else if (lParam==0)
	{ mmix_status(MMIX_STOPPED);
      show_stop_marker(edit_file_no,-1); /* clear stop marker */
	}
	else
	{ mmix_status(MMIX_STOPPED);
	  show_stop_marker(item_file_no(lParam),item_line_no(lParam));
	}
	update_profile();
	memory_update();
	return 0;
  case WM_MMIX_RESET:
	return 0;
  case WM_MMIX_LOAD:
    update_breakpoints();
	return 0;
  case WM_CLOSE:
	if (ed_close_all(1))
	  DestroyWindow(hWnd);
	return 0;
  case WM_DESTROY:
	ed_close_all(0);
	set_xypos(hMainWnd);
	write_regtab(defined);
    PostQuitMessage(0);
    return 0;
  case WM_SYSCOMMAND:
	//  	ed_close_all(0);
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
	wcex.hIcon			= LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_MENUBAR+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MENU);
	wcex.lpszClassName = classname;
	wcex.hIconSm		= LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON));

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

#define ED_BLOCKSIZE 0x800
void init_edit(HINSTANCE hInstance)
{	Scintilla_RegisterClasses(hInstance);
	Scintilla_LinkLexers();
}


LONG_PTR ed_send(unsigned int msg,ULONG_PTR wparam,LONG_PTR lparam)
{  if (hEdit==NULL) return 0;
	return ed_fn(ed_ptr,msg,wparam,lparam);
  // Scintilla_DirectFunction
}

#define MMIXAL_COLORS (SCE_MMIXAL_INCLUDE+1)
COLORREF syntax_color[MMIXAL_COLORS] =
{       
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_LEADWS 0
  RGB(0x00,0xB0,0x00),		//SCE_MMIXAL_COMMENT 1
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_LABEL 2
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_OPCODE 3
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_OPCODE_PRE 4
  RGB(0x80,0x00,0x80),		//SCE_MMIXAL_OPCODE_VALID 5
  RGB(0xFF,0x00,0x00),		//SCE_MMIXAL_OPCODE_UNKNOWN 6
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_OPCODE_POST 7
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_OPERANDS 8
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_NUMBER 9
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_REF 10
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_CHAR 11
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_STRING 12
  RGB(0x00,0x00,0xFF),		//SCE_MMIXAL_REGISTER 13
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_HEX 14
  RGB(0x00,0x00,0x00),		//SCE_MMIXAL_OPERATOR 15
  RGB(0x00,0x80,0xFF),		//SCE_MMIXAL_SYMBOL 16
  RGB(0x00,0x00,0x00)		//SCE_MMIXAL_INCLUDE 17
};


void mms_style(void)
/* configure the MMIXAL lexer */
{  int stylebits;  
   ed_send(SCI_SETLEXER,SCLEX_MMIXAL,0);
   stylebits= (int)ed_send(SCI_GETSTYLEBITSNEEDED,0,0);
   ed_send(SCI_SETSTYLEBITS,stylebits,0);
   
   ed_send(SCI_SETKEYWORDS,0,(sptr_t)mmix_opcodes);
   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_OPCODE_VALID,syntax_color[SCE_MMIXAL_OPCODE_VALID]);
   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_OPCODE_UNKNOWN,syntax_color[SCE_MMIXAL_OPCODE_UNKNOWN]);

   ed_send(SCI_SETKEYWORDS,1,(sptr_t)mmix_special_registers);
   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_REGISTER,syntax_color[SCE_MMIXAL_REGISTER]);

   ed_send(SCI_SETKEYWORDS,2,(sptr_t)mmix_predefined);
   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_SYMBOL,syntax_color[SCE_MMIXAL_SYMBOL]);

   ed_send(SCI_STYLESETFORE,SCE_MMIXAL_COMMENT,syntax_color[SCE_MMIXAL_COMMENT]);
   syntax_highlighting = 1;
}

void txt_style(void)
/* configure the NULL lexer */
{  int stylebits;  
   ed_send(SCI_SETLEXER,SCLEX_NULL,0);
   stylebits= (int)ed_send(SCI_GETSTYLEBITSNEEDED,0,0);
   ed_send(SCI_SETSTYLEBITS,stylebits,0);
   ed_send(SCI_STYLESETFORE,0,RGB(0x0,0x0,0x0));
   ed_send(SCI_STYLESETBACK,0,RGB(0xFF,0xFF,0xFF));
   ed_send(SCI_COLOURISE,0,-1);
   syntax_highlighting = 0;
}

void set_lineno_width(void)
{ int width;
  if (show_line_no)
  { char zeros[]= "00000";
    char *number;
    int lines;
    lines=(int)ed_send(SCI_GETLINECOUNT,0,0);
    if (lines<100) number= zeros+2;
    else if (lines<1000) number = zeros+1;
    else number=zeros;
    width =(int) ed_send(SCI_TEXTWIDTH,MMIX_LINE_MARGIN,(sptr_t)number);
  }
  else
    width = 0;
  ed_send(SCI_SETMARGINWIDTHN,MMIX_LINE_MARGIN,width);
}

void set_profile_width(void)
{ int width;
  if (show_profile)
    width =(int) ed_send(SCI_TEXTWIDTH,MMIX_PROFILE_MARGIN,(sptr_t)"000000");
  else
    width = 0;
  ed_send(SCI_SETMARGINWIDTHN,MMIX_PROFILE_MARGIN,width);
}

#define STYLE_PROFILE (STYLE_LASTPREDEFINED+1)

void update_profile(void)
{ int line_no, from, to;
  COLORREF back;
  if (!show_profile) return;

  from = (int)ed_send(SCI_GETFIRSTVISIBLELINE,0,0);
  to= from+(int)ed_send(SCI_LINESONSCREEN,0,0);

  ed_send(SCI_STYLESETFORE,STYLE_PROFILE, RGB(0x00,0x00,0xD0));
  back = (COLORREF)ed_send(SCI_STYLEGETBACK,STYLE_LINENUMBER,0);
  ed_send(SCI_STYLESETBACK,STYLE_PROFILE, back);
  for (line_no=from;line_no<=to; line_no++)
  { char number[21];
    int freq = line2freq(edit_file_no,line_no+1);
	if (freq<=0) continue;
    sprintf(number,"%d",freq);
	ed_send(SCI_MARGINSETTEXT,line_no, (sptr_t)number);
    ed_send(SCI_MARGINSETSTYLE,line_no, STYLE_PROFILE);
  }

}

void set_tabwidth(void)
{   ed_send(SCI_SETTABWIDTH,tabwidth,0);
}


void create_edit(void)
{   sp_create_options(1,0,0.8,0,NULL);
   hEdit = CreateWindowEx(0,"Scintilla","Editor",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN,
		0,0,400,300,hSplitter,NULL,hInst,NULL);
   ed_fn = (SciFnDirect)SendMessage(hEdit,SCI_GETDIRECTFUNCTION,0,0);
   ed_ptr= (sptr_t)SendMessage(hEdit,SCI_GETDIRECTPOINTER,0,0);
}

void new_edit(void)
{ 
  if (hEdit==NULL) create_edit();
  /* configure the style */
   ed_send(SCI_STYLESETFONT,STYLE_DEFAULT,(sptr_t)"Courier New");
   ed_send(SCI_STYLESETSIZE,STYLE_DEFAULT,(sptr_t)12);
   ed_send(SCI_STYLESETBOLD,STYLE_DEFAULT,(sptr_t)1);
   ed_send(SCI_SETVISIBLEPOLICY,CARET_SLOP|CARET_JUMPS|CARET_EVEN,3);
   set_text_style();
   /* configure margins and markers */
   /* line numbers */
   ed_send(SCI_SETMARGINTYPEN,MMIX_LINE_MARGIN,SC_MARGIN_NUMBER);
   set_lineno_width();

   /* profile margin */
   ed_send(SCI_SETMARGINTYPEN,MMIX_PROFILE_MARGIN,SC_MARGIN_RTEXT);
   ed_send(SCI_SETMARGINMASKN,MMIX_PROFILE_MARGIN,0);

   ed_send(SCI_STYLESETFORE,STYLE_PROFILE, RGB(0x80,0xFF,0x80));

   set_profile_width();
   update_profile();

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
   /* set standatd encoding */
   SendMessage(hMainWnd,WM_COMMAND,(WPARAM)ID_ENCODING_ASCII,(LPARAM)0);
}

void update_breakpoints(void)
{ int line = -1;
  if (application_file_no<0) return;
  if (break_at_Main) break_at_symbol(":Main");
  if (hEdit==NULL) return;
  set_edit_file(application_file_no);
  mem_clear_breaks(edit_file_no);
  while ((line = (int)ed_send(SCI_MARKERNEXT,line+1,(1<<MMIX_BREAKX_MARKER)))>=0)
   if (!set_breakpoint(edit_file_no,line+1))
     ed_send(SCI_MARKERDELETE,line,MMIX_BREAKX_MARKER);
}

void ed_new(void)
{ int file_no;
  if (hEdit==NULL) new_edit();
  file_no = filename2file(NULL);
  set_edit_file(file_no);
/*	ed_send(SCI_SETSAVEPOINT,0,0);
	ed_send(SCI_CANCEL,0,0);
*/
}

int ed_close(void)
/* return 1 if file was closed zero otherwise */
{ int file_no;
  if (!ed_save_changes(1)) return 0;
  close_file(edit_file_no);
  edit_file_no=-1; /* no edit file yet */
  file_no=get_inuse_file();
  if (file_no>=0) set_edit_file(file_no);
  else ed_new();
  return 1;
}

int ed_close_all(int cancel)
/* return 1 if all files could be closed zero otherwise */
{ int file_no;
  file_no=edit_file_no;
  while(edit_file_no>=0)
  { if (!ed_save_changes(cancel)) return 0;
    close_file(edit_file_no);
    edit_file_no=-1; /* no edit file yet */
    file_no=get_inuse_file();
    if (file_no>=0) set_edit_file(file_no);
  }
  ed_new();
  return 1;
}



int ed_open_file(void)
{ FILE *fp;
  fp = fopen(file2fullname(edit_file_no), "rb");
  if (fp) {
    char data[ED_BLOCKSIZE];
    size_t len;
    while ((len = fread(data, 1, ED_BLOCKSIZE, fp))>0)
      ed_send(SCI_ADDTEXT, len, (sptr_t)data);
	fclose(fp); 
	file2dirty(edit_file_no)=0;
    ed_send(SCI_SETUNDOCOLLECTION, 1,0);
    ed_send(SCI_SETSAVEPOINT,0,0);
    ed_send(SCI_GOTOPOS,0,0);
	set_lineno_width();
	return edit_file_no;
  }
  else
  { ide_status("Unable to open file");
    return -1;
  }
}

int ed_open(void)
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
      return filename2file(name);
	else
      return -1;
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

int mmo_file_newer(char *full_mms_name)
{ HANDLE mms, mmo;
  FILETIME mmsTime, mmoTime;
  char *mmo_name;
  if (full_mms_name==NULL || full_mms_name[0]==0) return 0;
  mms = CreateFile(full_mms_name,FILE_READ_ATTRIBUTES,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (mms==INVALID_HANDLE_VALUE) return 0;
  mmo_name = get_mmo_name(full_mms_name);
  if (mmo_name==NULL) return 0;
  mmo = CreateFile(mmo_name,FILE_READ_ATTRIBUTES,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (mmo==INVALID_HANDLE_VALUE) return 1;
  GetFileTime(mms,NULL,NULL,&mmsTime);
  GetFileTime(mmo,NULL,NULL,&mmoTime);
  if (CompareFileTime(&mmsTime,&mmoTime)>0)
	  return 1;
  else
	  return 0;
}



void ed_save(void)
{ if (hEdit==NULL) return;
  if (file2fullname(edit_file_no)==0)
    ed_save_as();
  else
	ed_save_name(file2fullname(edit_file_no));
}

void ed_save_as(void)
{	char *name=NULL;
	char asname[MAX_PATH+1] = "\0";
	OPENFILENAME ofn ={0};
	if (hEdit==NULL) return;
	if (edit_file_no>=0)
	  name = file2fullname(edit_file_no);
	if (name!=NULL)
	  strncpy(asname,name ,MAX_PATH);
	else
	  asname[0]=0;
	ofn.lStructSize= sizeof(ofn);
	ofn.lpstrFile = asname;
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFile = asname;
	ofn.lpstrFilter = "MMIX (.mms)\0*.mms\0" "All Files (*.*)\0*.*\0\0";
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Save File";
	ofn.Flags = OFN_HIDEREADONLY;

	if (GetSaveFileName(&ofn) && !dont_overwrite(asname)) 
	{  file_set_name(edit_file_no,asname);
	   ed_save_name(file2fullname(edit_file_no));
	  //InvalidateRect(hEdit, NULL, NULL);
	}
}

int ed_save_changes(int cancel)
{ int dirty;
  if (hEdit==NULL) return 1;
  dirty = (int)ed_send(SCI_GETMODIFY,0,0);
  if (dirty)
  {	 
     if (autosave)
	 {  ed_save();
	    return 1;
	 }
	 else 
	 { int decision= MessageBox(hMainWnd, "Save changes ?", file_listname(edit_file_no), cancel?MB_YESNOCANCEL:MB_YESNO);
	   if (decision == IDYES)
	     ed_save();
	   else if (decision == IDCANCEL)
	     return 0;
	 }
  }
  return 1;
}

int auto_assemble=0;

int assemble_if_needed(int file_no)
{ char *full_mms_name=file2fullname(file_no);
  if (mmo_file_newer(full_mms_name))
  {	 int decision;
     if (auto_assemble)
    	 decision= IDYES;
	 else	 
		 decision= MessageBox(hMainWnd, "mms file newer than mmo file. Assemble?", file_listname(file_no), MB_YESNOCANCEL);
	 if (decision == IDYES)
	 { int err_count=mmix_assemble(file_no);
	   if (err_count!=0) return 0;
	 } 
	 else if (decision == IDCANCEL)
	   return 0;
  }
  else if (!file2debuginfo(file_no))
  {	 int decision;
     if (auto_assemble)    	 
		 decision= IDYES;
	 else
		 decision= MessageBox(hMainWnd, "no debug information available. Assemble?", file_listname(file_no), MB_YESNOCANCEL);
	 if (decision == IDYES)
	 { int err_count=mmix_assemble(file_no);
	   if (err_count!=0) return 0;
	 } 
	 else if (decision == IDCANCEL)
	   return 0;
  }
  return 1;
}

static int all_assembled;

static void assemble_loading_files(int file_no)
{ if (!all_assembled) return;
  if (!file2loading(file_no)) return;
  if (!assemble_if_needed(file_no)) all_assembled=0;
}

int assemble_all_needed(void)
{ all_assembled=1;
  for_all_files(assemble_loading_files);
  return all_assembled && assemble_if_needed(edit_file_no);
}

void new_errorlist(void)
{   sp_create_options(0,0,0.2,0,hEdit);
	hError = CreateWindow("LISTBOX", "Errorlist" ,
				WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT 
,
                0,0,0,0,
	            hSplitter, NULL, hInst, NULL);
}


void add_button(int iconID, int menuID, int group, int buttonID, char *tip)
{ HANDLE h;
  h = LoadImage(hInst, MAKEINTRESOURCE(iconID),IMAGE_ICON,0,0,
	     LR_DEFAULTCOLOR);
  bb_CreateButton(hButtonBar,h,menuID,group,buttonID,1,1,tip);
}

void add_buttons(void)
{ 
  add_button(IDI_FILE_NEW,ID_FILE_NEW,BG_FILE,0,"New");
  add_button(IDI_FILE_OPEN,ID_FILE_OPEN,BG_FILE,1,"Open");
  add_button(IDI_FILE_SAVE,ID_FILE_SAVE,BG_FILE,2,"Save");
  
  add_button(IDI_EDIT_COPY,ID_EDIT_COPY,BG_EDIT,3,"Copy");
  add_button(IDI_EDIT_PASTE,ID_EDIT_PASTE,BG_EDIT,4,"Paste");
  add_button(IDI_EDIT_CUT,ID_EDIT_CUT,BG_EDIT,5,"Cut");
  add_button(IDI_EDIT_UNDO,ID_EDIT_UNDO,BG_EDIT,6,"Undo");
  add_button(IDI_EDIT_REDO,ID_EDIT_REDO,BG_EDIT,7,"Redo");
  add_button(IDI_FINDREPLACE,ID_EDIT_FIND,BG_EDIT,8,"Find and Replace");
  
  add_button(IDI_VIEW_ZOOMIN,ID_VIEW_ZOOMIN,BG_VIEW,9,"Zoom in");
  add_button(IDI_VIEW_ZOOMOUT,ID_VIEW_ZOOMOUT,BG_VIEW,10,"Zoom out");
  add_button(IDI_VIEW_WHITESPACE,ID_VIEW_WHITESPACE,BG_VIEW,11,"Show Whitespace");

  add_button(IDI_MMIX_DEBUG,ID_MMIX_DEBUG,BG_MMIX,12,"Debug/Continue");
  add_button(IDI_DEBUG_STEP,ID_MMIX_STEP,BG_DEBUG,13,"Step Instruction");
  add_button(IDI_DEBUG_PAUSE,ID_MMIX_STOP,BG_DEBUG,15,"Break Execution");
  add_button(IDI_DEBUG_HALT,ID_MMIX_QUIT,BG_DEBUG,16,"Halt Execution");
  bb_set_group(hButtonBar,BG_DEBUG,0,0);

  add_button(IDI_HELP,ID_HELP_ABOUT,BG_HELP,17,"About");


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
    printer_init();
	init_edit(hInstance);
	mmix_lib_initialize();
	debug_init();
	  param_init ();
	  read_regtab(defined);
	  get_xypos();
      SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);

	new_edit();
	if (edit_file_no<0) ed_new();
	hStatus = CreateWindow("STATIC", version ,
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
	if (vmb.connected) vmb_atexit();
	return (int)msg.wParam;
}

void  ide_exit_ignore(int returncode)
{
;
}