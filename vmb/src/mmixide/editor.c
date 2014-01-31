#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#pragma warning(disable : 4996)
#include "resource.h"
#include "winmain.h"
#include "splitter.h"
#include "info.h"
#include "debug.h"
#include "mmixdata.h"
#include "edit.h"
#include "editor.h"
#define STATIC_BUILD
#include "../scintilla/include/scintilla.h"
#include "../scintilla/include/scilexer.h"



int edit_file_no = -1; /* the file currently in the editor */

HWND hEdit=NULL;
static HWND hSCe=NULL;
static HWND hTabs=NULL;
static int tabh = 40;
static LRESULT CALLBACK EditorProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int register_editor(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;
  ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)EditorProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	=  (HBRUSH)(COLOR_APPWORKSPACE+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName = "MMIXEDITORCLASS";
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}


static void create_edit(void)
{  
   sp_create_options(1,0,0.8,0,NULL);
   hEdit = CreateWindow("MMIXEDITORCLASS", NULL ,
				WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN|WS_BORDER,
                CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,
	            hSplitter, NULL, hInst, NULL);
}



SciFnDirect ed_fn;
sptr_t ed_ptr;


LONG_PTR ed_send(unsigned int msg,ULONG_PTR wparam,LONG_PTR lparam)
{  if (hSCe==NULL) return 0;
	return ed_fn(ed_ptr,msg,wparam,lparam);
  // Scintilla_DirectFunction used only in findreplace.c and print.c
}

void  ed_operation(unsigned int op)
{ ed_send(op,0,0);
}

void ed_tab_select(int file_no);
#define ED_BLOCKSIZE 0x800
static void ed_read_file(void)
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
  }
  else
  { ide_status("Unable to open file");
  }
  file2reading(edit_file_no)=0;
}


void set_edit_file(int file_no)
{ if (hEdit==NULL) new_edit();
  if (file_no==edit_file_no || file_no<0) return;
  ide_clear_error_marker();
  clear_stop_marker();
  if (edit_file_no>=0)
    file2dirty(edit_file_no)=(int)ed_send(SCI_GETMODIFY,0,0);
  edit_file_no = file_no;
  ed_send(SCI_SETDOCPOINTER,0,(LONG_PTR)doc[edit_file_no]);
  if (fullname[edit_file_no]!=NULL && file2reading(edit_file_no)) ed_read_file();
  set_tabwidth();
  set_text_style();
  set_whitespace(show_whitespace);
  ed_send(SCI_SETCODEPAGE,codepage,0);
  //SetWindowText(hMainWnd,unique_name(file_no));
  ed_tab_select(edit_file_no);
  //file_list_mark(edit_file_no);
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




void init_edit(HINSTANCE hInstance)
{	Scintilla_RegisterClasses(hInstance);
	Scintilla_LinkLexers();
    register_editor(hInst);	
}

static LRESULT CALLBACK EditorProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 	
  switch (message) 
  { case WM_CREATE:
         hSCe = CreateWindowEx(WS_EX_LEFT,"Scintilla","Editor",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN,
		       0,0,0,0,hWnd,NULL,hInst,NULL);
         ed_fn = (SciFnDirect)SendMessage(hSCe,SCI_GETDIRECTFUNCTION,0,0);
         ed_ptr= (sptr_t)SendMessage(hSCe,SCI_GETDIRECTPOINTER,0,0);
		 hTabs = CreateWindowEx(WS_EX_LEFT,WC_TABCONTROL,"EditorTabs",WS_CHILD|WS_TABSTOP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
		       0,0,0,0,hWnd,NULL,hInst,NULL);
         SendMessage(hTabs,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),0);
		 
	     return 0;
    case WM_SIZE:
		if (wParam==SIZE_RESTORED || SIZE_MAXIMIZED)
		{ if (TabCtrl_GetItemCount(hTabs)>1)
		  {  RECT r;
		     r.top=r.left=0;
		     r.bottom=HIWORD(lParam);
		     r.right=LOWORD(lParam);
		     TabCtrl_AdjustRect(hTabs,FALSE,&r);
			 ShowWindow(hTabs,SW_SHOW);
		     MoveWindow(hTabs,0,0,LOWORD(lParam),HIWORD(lParam), TRUE);
             MoveWindow(hSCe,r.left,r.top,r.right-r.left,r.bottom-r.top, TRUE);
		   }
		   else
		   { ShowWindow(hTabs,SW_HIDE);
		     MoveWindow(hSCe,0,0,LOWORD(lParam),HIWORD(lParam), TRUE);
		   }
		}
		break;
	case WM_DESTROY:
		ed_close_all(0);		  
		hEdit=NULL;
		hSCe=NULL;
		hTabs=NULL;
		break;
    case WM_NOTIFY:
	  { NMHDR *n = (NMHDR*)lParam;
	    if (n->hwndFrom==hTabs)
		{ if (n->code == TCN_SELCHANGE)
		  { int index = TabCtrl_GetCurSel(hTabs);
 			TCITEM tie;
			tie.mask=TCIF_PARAM;
			if (TabCtrl_GetItem(hTabs,index,&tie))
			  set_edit_file((int)tie.lParam);
		  }
		}
	    else if (n->hwndFrom=hSCe)
	    {  if (n->code == SCN_MARGINCLICK)
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
		    { if (set_breakpoint(edit_file_no,line+1))
		        ed_send(SCI_MARKERADD,line,MMIX_BREAKX_MARKER);
		    }
		  }
		  else if(n->code==SCN_MODIFIED)
		  { struct SCNotification *sn=(struct SCNotification*)n;
		    if (sn->linesAdded!=0) clear_stop_marker();
		  }
	    }
	  return 0;
	}
  }
  return (DefWindowProc(hWnd, message, wParam, lParam));
}



void ed_zoom_in(void)
{ ed_send(SCI_ZOOMIN,0,0);
  set_lineno_width();
}
void ed_zoom_out(void)
{  ed_send(SCI_ZOOMOUT,0,0);
  set_lineno_width();
}

void ed_set_ascii(void)
{ ed_send(SCI_SETCODEPAGE,0,0);
  codepage=0;
}

void ed_set_utf8(void)
{ ed_send(SCI_SETCODEPAGE,SC_CP_UTF8,0);
  codepage=SC_CP_UTF8;
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

#define ERROR_MARKER 0

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



int ed_open(void)
/* opens a file as a new document */
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


static void ed_write_file(void)
{	FILE *fp;
    char *name = fullname[edit_file_no];
    if (name==NULL || name[0]==0) return;
    fp = fopen(name, "wb");
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


void ed_save(void)
{ if (hEdit==NULL) return;
  if (file2fullname(edit_file_no)==NULL)
    ed_save_as();
  else
	ed_write_file();
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
	   ed_write_file();
	   file2reading(edit_file_no)=0;
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
	 { int decision= MessageBox(hMainWnd, "Save changes ?", unique_name(edit_file_no), cancel?MB_YESNOCANCEL:MB_YESNO);
	   if (decision == IDYES)
	     ed_save();
	   else if (decision == IDCANCEL)
	     return 0;
	 }
  }
  return 1;
}



static int previous_error_line_no=-1;


void ide_clear_error_marker(void)
{ ed_send(SCI_MARKERDELETEALL,ERROR_MARKER,0);
  previous_error_line_no=-1;
}

void ed_mark_error(int file_no, int line_no)
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
  {  SendMessage(hSCe,SCI_MARKERDELETEALL,MMIX_TRACE_MARKER,0);  
     previous_mmix_line_no= -1;
  }
}

void show_stop_marker(int file_no, int line_no)
{ set_edit_file(file_no);
  if (previous_mmix_line_no > 0)
	PostMessage(hSCe,SCI_MARKERDELETE,previous_mmix_line_no-1,MMIX_TRACE_MARKER);
  if (line_no>0) {
    PostMessage(hSCe,SCI_MARKERADD, line_no-1, MMIX_TRACE_MARKER);
	PostMessage(hSCe,SCI_ENSUREVISIBLEENFORCEPOLICY,line_no-1,0); 
//	PostMessage(hSCe,SCI_GOTOLINE,line_no-1,0); /* first line is 0 */
  }
  previous_mmix_line_no=line_no;
}

void ide_clear_mmix_line(void)
{  ed_send(SCI_MARKERDELETEALL,MMIX_TRACE_MARKER,0);
   previous_mmix_line_no=-1;
}


int fontsize;
int show_whitespace=0;
int syntax_highlighting = 0;
int codepage=0;

void set_whitespace(int ws)
{ show_whitespace=ws;
	if (show_whitespace)
		  {  ed_send(SCI_SETVIEWWS,SCWS_VISIBLEALWAYS,0);
		     ed_send(SCI_SETVIEWEOL,1,0);
		  }
  		  else
		  {  ed_send(SCI_SETVIEWWS,SCWS_INVISIBLE,0);
		     ed_send(SCI_SETVIEWEOL,0,0);
		  }
}

void ed_refresh_breaks(void)
{ int line=-1;
  mem_clear_breaks(edit_file_no);
  while ((line = (int)ed_send(SCI_MARKERNEXT,line+1,(1<<MMIX_BREAKX_MARKER)))>=0)
   if (!set_breakpoint(edit_file_no,line+1))
     ed_send(SCI_MARKERDELETE,line,MMIX_BREAKX_MARKER);
}

void  ed_show_line(int line_no)
{  ed_send(SCI_ENSUREVISIBLEENFORCEPOLICY,line_no-1,0); 
   //  ed_send(SCI_GOTOLINE,line_no-1,0); /* first line is 0 */
   SetFocus(hSCe);
}

void mms_style(void);
void txt_style(void);

void set_text_style(void)
{ if (syntax_highlighting)
     mms_style();
  else
     txt_style();
}

void *ed_create_document(void)
{ return (void *)ed_send(SCI_CREATEDOCUMENT,0,0);
}

void ed_release_document(void * doc)
{ if (doc!=NULL) ed_send(SCI_RELEASEDOCUMENT,0,(LONG_PTR)doc);
}




static int find_tab(int file_no)
/* searches the tab control for the given file_no. Returns teh index if found, -1 otherwise. */
{ int count = TabCtrl_GetItemCount(hTabs);
  int index;
  for (index=0; index<count;index++)
  { TCITEM tie;
    tie.mask=TCIF_PARAM;
	if (TabCtrl_GetItem(hTabs,index,&tie) && tie.lParam==file_no) return index;
  }
  return -1;
}

static void resize_tab(void)
{ RECT r;
  GetWindowRect(hEdit,&r);
  SendMessage(hEdit,WM_SIZE,SIZE_RESTORED,MAKELONG( r.right-r.left,r.bottom-r.top));
}

void ed_tab_select(int file_no)
{  int index = find_tab(file_no);
   if (index>=0)
	 TabCtrl_SetCurSel(hTabs,index);
}

void ed_remove_tab(int file_no)
{ int index = find_tab(file_no);
  if (index>=0)
  {  TabCtrl_DeleteItem (hTabs, index);
     if (TabCtrl_GetItemCount(hTabs)==1) resize_tab();
  }
}

void ed_add_tab(int file_no)
{ TCITEM tie;
  int index;
  if (hTabs==NULL) return;
  if (file_no<0) return;
  index = find_tab(file_no);
  if (index>=0)
  { tie.mask = TCIF_TEXT|TCIF_PARAM;
    tie.pszText = unique_name(file_no);
    tie.lParam=file_no;
    TabCtrl_SetItem (hTabs, file_no, &tie);
  }
  else
  { index = TabCtrl_GetItemCount(hTabs);
    tie.mask = TCIF_TEXT|TCIF_PARAM;
    tie.pszText = unique_name(file_no);
    tie.lParam=file_no;
    TabCtrl_InsertItem (hTabs, index, &tie);
	if (index==1) resize_tab();
  }
  TabCtrl_SetCurSel (hTabs, index);
}
