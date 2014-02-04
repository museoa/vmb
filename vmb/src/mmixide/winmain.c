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
#include "symtab.h"
#include "mmixdata.h"
#include "edit.h"
#include "assembler.h"
#include "sources.h"
#include "debug.h"
#include "breakpoints.h"
#include "editor.h"
#include "winlog.h"
#include "winmain.h"
#define STATIC_BUILD
#include "../scintilla/include/scintilla.h"

int major_version=1, minor_version=0;
char version[]="$Revision: 1.23 $ $Date: 2014-02-04 16:45:11 $";
char title[] ="VMB MMIX IDE";

/* Button groups for the button bar */
#define BG_FILE 0
#define BG_EDIT 1
#define BG_VIEW 2
#define BG_MMIX 3
#define BG_DEBUG 4
#define BG_HELP 5


HINSTANCE hInst;
HWND	  hMainWnd;
HMENU	  hMenu;
HWND	  hSplitter;
HWND      hButtonBar;
HWND      hStatus;

HWND	  hError=NULL;
HWND	  hSymbolTable=NULL;

#define S_WIDTH 200

void ide_status(char *message)
{ SendMessage(hStatus, WM_SETTEXT,0,(LPARAM)message);
  UpdateWindow(hStatus);
  //SetWindowText(hStatus,message);
}



void ide_add_error(char *message, int file_no, int line_no)
{ int item;
  if (hError==NULL) new_errorlist();
  item = (int) SendMessage(hError,LB_ADDSTRING,0,(LPARAM)message);
  if (item==LB_ERR||item==LB_ERRSPACE) return;
  SendMessage(hError,LB_SETITEMDATA,(WPARAM)item,item_data(file_no, line_no));
}


void ide_clear_error_list(void)
/* clear error markers in current edit file */
{   if (hError==NULL) return;
	SendMessage(hError,LB_RESETCONTENT ,0,0);
	ide_clear_error_marker();
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





int ide_prepare_mmix(void)
{  if (mmix_active())
  { MessageBox(hMainWnd,"MMIX is already running.","Error",MB_OK|MB_ICONEXCLAMATION);
    return 0;
  }
  if (!ed_save_all(1)) return 0;
  if (!assemble_all_needed()) return 0;
  if (!ide_connect()) return 0;
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
		  else if (hChildWnd==hSymbolTable)
		  { CheckMenuItem(hMenu,ID_VIEW_SYMBOLTABLE,MF_BYCOMMAND|MF_UNCHECKED);
		    hSymbolTable=NULL;
		  }
		}
		return 1;

  case WM_MEASUREITEM:
	if (wParam==GetWindowLong(hSymbolTable,GWL_ID))
		return symtab_measureitem((LPMEASUREITEMSTRUCT)lParam);
	else
	    break;
  case WM_DRAWITEM: 
    if (wParam==GetWindowLong(hSymbolTable,GWL_ID))
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
		  ed_operation(SCI_UNDO);
		  return 0;
	    case ID_EDIT_REDO:
		  ed_operation(SCI_REDO);
		  return 0;
	    case ID_EDIT_CUT:
		  ed_operation(SCI_CUT);
		  return 0;
	    case ID_EDIT_COPY:
		  ed_operation(SCI_COPY);
		  return 0;
	    case ID_EDIT_PASTE:
		  ed_operation(SCI_PASTE);
		  return 0;
	    case ID_EDIT_DELETE:
		  ed_operation(SCI_CLEAR);
		  return 0;
	    case ID_EDIT_SELECTALL:
		  ed_operation(SCI_SELECTALL);
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
  	    case ID_VIEW_TRACE:
		  show_trace_window();
		  return 0;
	    case ID_VIEW_ZOOMIN:
		  ed_zoom_in();
		  return 0;
	    case ID_VIEW_ZOOMOUT:
		  ed_zoom_out();
		  return 0;
		case ID_VIEW_WHITESPACE:
          set_whitespace(menu_toggle(ID_VIEW_WHITESPACE));
		  return 0;
		case ID_ENCODING_ASCII:
		  ed_set_ascii();
          CheckMenuItem(hMenu,ID_ENCODING_ASCII,MF_BYCOMMAND|MF_CHECKED);
          CheckMenuItem(hMenu,ID_ENCODING_UTF,MF_BYCOMMAND|MF_UNCHECKED);
          return 0;
		case ID_ENCODING_UTF:
		  ed_set_utf8();
          CheckMenuItem(hMenu,ID_ENCODING_ASCII,MF_BYCOMMAND|MF_UNCHECKED);
          CheckMenuItem(hMenu,ID_ENCODING_UTF,MF_BYCOMMAND|MF_CHECKED);
          return 0;
		case ID_VIEW_SYNTAX:
          syntax_highlighting=menu_toggle(ID_VIEW_SYNTAX);
		  set_text_style();
		  return 0;
		case ID_MMIX_STEP:
		  mmix_continue('s');
		  return 0;
		case ID_MMIX_STEPOVER:
		  mmix_continue('n');
		  return 0;
		case ID_MMIX_STEPOUT:
		  mmix_continue('o');
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
		    mmix_debug();
		  }
	      return 0; 
		case ID_MMIX_RUN:
		  if (mmix_active()) return 0;
		  if (!ide_prepare_mmix()) return 0;
		  mmix_run();
	      return 0; 

	    case ID_MMIX_ASSEMBLE:
		  if (!ed_save_changes(1)) return 0;
		  if (mmix_assemble(edit_file_no)==0 && edit_file_no==application_file_no && mmix_active() && vmb.power)
		  { MessageBox(hWnd,"mmo file already running! Reset to reload file.", unique_name(edit_file_no),MB_OK|MB_ICONWARNING);
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
			ed_mark_error(file_no,line_no);
			return 0;
		  case   LBN_SELCANCEL:
  			ed_mark_error(-1,-1);
			return 0;
		  case LBN_SETFOCUS:
			  break;
		   case LBN_KILLFOCUS:
			  break;
		  default:
			  return 0;
		}
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
		  ed_show_line(line_no);
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
  /* change color of edit contols like trace */
  case WM_CTLCOLOREDIT :
  case WM_CTLCOLORSTATIC:
		{ HDC hdc = (HDC)wParam;
		  HWND hc = (HWND)lParam;
		  if (hc==hLog)
		  { SetTextColor(hdc,RGB(0xff,0xff,0xff));
		    SetBkColor(hdc,RGB(0,0,0));
		    return (LRESULT)GetStockObject(BLACK_BRUSH);
		  }
		  else
			 break;
		}
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




void update_breakpoints(void)
{ int line = -1;
  if (application_file_no<0) return;
  if (break_at_Main) break_at_symbol(":Main");
  if (hEdit==NULL) return;
  set_edit_file(application_file_no);
  ed_refresh_breaks();
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





int assemble_if_needed(int file_no)
{ char *full_mms_name;
  if (file_no<0) return 1;
  full_mms_name=file2fullname(file_no);
  if (full_mms_name==NULL)
  { set_edit_file(file_no);
    if (!ed_save_as()) 
	  return 0;
  }
  if (mmo_file_newer(full_mms_name))
  {	 int decision;
     if (auto_assemble)
    	 decision= IDYES;
	 else	 
		 decision= MessageBox(hMainWnd, "mms file newer than mmo file. Assemble?", unique_name(file_no), MB_YESNOCANCEL);
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
		 decision= MessageBox(hMainWnd, "No debug information available. Assemble?", unique_name(file_no), MB_YESNOCANCEL);
	 if (decision == IDYES)
	 { int err_count=mmix_assemble(file_no);
	   if (err_count!=0) return 0;
	 } 
	 else if (decision == IDCANCEL)
	   return 0;
  }
  return 1;
}

static int assembly_ok=0;

static void assemble_loading_files(int file_no)
{ if (!assembly_ok) return;
  if (!file2assembly(file_no)) return;
  if (file_no==application_file_no) return;
  if (!assemble_if_needed(file_no)) assembly_ok=0;
}

int assemble_all_needed(void)
{ if (application_file_no<0&& missing_app)
  { MessageBox(hMainWnd, "No mms file selected as application.", "Warning", MB_ICONEXCLAMATION|MB_OK);
  }
  assembly_ok=1;
  for_all_files(assemble_loading_files);
  return assembly_ok && assemble_if_needed(application_file_no);
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
  add_button(IDI_DEBUG_STEPOVER,ID_MMIX_STEPOVER,BG_DEBUG,14,"Step Over");
  add_button(IDI_DEBUG_STEPOUT,ID_MMIX_STEPOUT,BG_DEBUG,15,"Step Out");
  add_button(IDI_DEBUG_PAUSE,ID_MMIX_STOP,BG_DEBUG,16,"Break Execution");
  add_button(IDI_DEBUG_HALT,ID_MMIX_QUIT,BG_DEBUG,17,"Halt Execution");
  bb_set_group(hButtonBar,BG_DEBUG,0,0);

  add_button(IDI_HELP,ID_HELP_ABOUT,BG_HELP,18,"About");


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