#include <windows.h>
#include "winopt.h"
#include "resource.h"
#include "winde.h"
#include "dedit.h"


/* DataEditor */
typedef struct dataedit
{  HWND hWnd;   /* the handle to the data edit dialog window */
   inspector_def *insp; /* the inspector connected to */
   char * reg_name; /* register name, if NULL this is memory */
   int reg_offset; /* offset to devices base address */
   uint64_t address; /* use instead of name and offset if name==NULL */
   unsigned char mem[8]; /* up to one octa */
   int size; /* 1,2,4, or 8  number of byte to edit */
   enum mem_fmt format; /* current format */
   enum chunk_fmt chunk; /* current chunk */
} dataedit;

static dataedit *new_dataedit(void)
{ dataedit *de;
  de = (dataedit*)malloc(sizeof(dataedit));
  if (de == NULL)
  { win32_error(__LINE__,"Out of Memory for data editor");
    return NULL;
  }
  memset(de,0,sizeof(dataedit));
  return de;
}

static void show_item(dataedit *de,int IDC,int offset)
{ char str[22]; 
  if (offset>=de->size) return;
  chunk_to_str(str, de->mem+offset,de->format,1<<de->chunk,0);
  SetDlgItemText(de->hWnd,IDC,str);
  SendMessage(GetDlgItem(de->hWnd,IDC),EM_SETMODIFY,0,0); 
}

static void show_edit_mem(dataedit *de)
{ if (de->chunk==byte_chunk)
  {	show_item(de,IDC_EDITBYTE0,0);
    show_item(de,IDC_EDITBYTE1,1);
    show_item(de,IDC_EDITBYTE2,2);
    show_item(de,IDC_EDITBYTE3,3);
    show_item(de,IDC_EDITBYTE4,4);
    show_item(de,IDC_EDITBYTE5,5);
    show_item(de,IDC_EDITBYTE6,6);
    show_item(de,IDC_EDITBYTE7,7);
  } else if (de->chunk==wyde_chunk)
  {	show_item(de,IDC_EDITWYDE0,0);
    show_item(de,IDC_EDITWYDE1,2);
    show_item(de,IDC_EDITWYDE2,4);
    show_item(de,IDC_EDITWYDE3,6);
 } else if (de->chunk==tetra_chunk)
  {	show_item(de,IDC_EDITTETRA0,0);
    show_item(de,IDC_EDITTETRA1,4);
  } else if (de->chunk==octa_chunk)
    show_item(de,IDC_EDITOCTA0,0);
}
static uint64_t str_to_u64(char *str)
{ uint64_t u=0;
  while (isspace(*str)) str++;
  while (*str!=0 && isdigit(*str))
  { u=u*10+((*str)-'0');
    str++;
  }
  return u;
}





static void str_to_chunk(char *str, unsigned char *buf, enum mem_fmt fmt, int chunk_size)
/* reads the data from str using format and chunk size.
   puts results into buffer.
*/
{ uint64_t u;
         
  if (fmt==float_format)
  { u=f64_from_str(str);
    if (chunk_size<8)
	  u=f32_from_f64(u);
  }
  else if (fmt==signed_format)
	u= (uint64_t)_atoi64(str);
  else if (fmt==unsigned_format)
    u= str_to_u64(str);
  else if (fmt==hex_format)
	u= hex_to_uint64(str);
  else if (fmt==ascii_format)
    u=(*str)&0xFF;

  while (chunk_size>0)
  { chunk_size--;
	*buf = (unsigned char)((u>>(8*chunk_size))&0xFF);
	buf++;
  }
}


static void put_item(dataedit *de, int IDC, int offset, int size)
{ char str[22]; 
  if (de->size>offset && SendMessage(GetDlgItem(de->hWnd,IDC),EM_GETMODIFY,0,0)) 
  { GetDlgItemText(de->hWnd,IDC,str,22);
	str_to_chunk(str, de->mem+offset,de->format,size);
  }
}

static void put_edit_mem(dataedit *de)
{ if (de->chunk==byte_chunk)
  {	put_item(de,IDC_EDITBYTE0,0,1);
	put_item(de,IDC_EDITBYTE1,1,1);
	put_item(de,IDC_EDITBYTE2,2,1);
	put_item(de,IDC_EDITBYTE3,3,1);
	put_item(de,IDC_EDITBYTE4,4,1);
	put_item(de,IDC_EDITBYTE5,5,1);
	put_item(de,IDC_EDITBYTE6,6,1);
	put_item(de,IDC_EDITBYTE7,7,1);
  }
  else if (de->chunk==wyde_chunk)
  {	put_item(de,IDC_EDITWYDE0,0,2);
	put_item(de,IDC_EDITWYDE1,2,2);
	put_item(de,IDC_EDITWYDE2,4,2);
	put_item(de,IDC_EDITWYDE3,6,2);
  }
  else if (de->chunk==tetra_chunk)
  {	put_item(de,IDC_EDITTETRA0,0,4);
	put_item(de,IDC_EDITTETRA1,4,4);
  }
  else if (de->chunk==octa_chunk)
	put_item(de,IDC_EDITOCTA0,0,8);
}
static void show_edit_windows(dataedit *de)
{ HWND h = de->hWnd;
  enum chunk_fmt chunk=de->chunk;
  int size=de->size;
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE0),chunk==byte_chunk&&size>0?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE1),chunk==byte_chunk&&size>1?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE2),chunk==byte_chunk&&size>2?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE3),chunk==byte_chunk&&size>3?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE4),chunk==byte_chunk&&size>4?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE5),chunk==byte_chunk&&size>5?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE6),chunk==byte_chunk&&size>6?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE7),chunk==byte_chunk&&size>7?SW_SHOW:SW_HIDE);

  ShowWindow(GetDlgItem(h,IDC_EDITWYDE0),chunk==wyde_chunk&&size>0?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITWYDE1),chunk==wyde_chunk&&size>2?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITWYDE2),chunk==wyde_chunk&&size>4?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITWYDE3),chunk==wyde_chunk&&size>6?SW_SHOW:SW_HIDE);

  ShowWindow(GetDlgItem(h,IDC_EDITTETRA0),chunk==tetra_chunk&&size>0?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITTETRA1),chunk==tetra_chunk&&size>4?SW_SHOW:SW_HIDE);

  ShowWindow(GetDlgItem(h,IDC_EDITOCTA0),chunk==octa_chunk&&size>0?SW_SHOW:SW_HIDE);
}

static set_edit_format(dataedit *de, enum mem_fmt format)
{ 
  if (format==ascii_format && de->chunk!=byte_chunk) format++;
  if (format>last_format) format=0;
  if (format==float_format && de->size<4) format=hex_format;
  if (format==float_format && de->chunk<tetra_chunk) format=hex_format;
  put_edit_mem(de);
  de->format=format;
  show_edit_mem(de);
  show_edit_windows(de);
  SetDlgItemText(de->hWnd, IDC_FORMAT, format_names[format]);
}
static set_edit_chunk(dataedit *de, enum chunk_fmt chunk)
{ if (chunk>last_chunk) chunk=byte_chunk;
  if (1<<chunk>de->size) chunk=byte_chunk;
  if (de->format==ascii_format)
   chunk=byte_chunk;
  else if (de->format==float_format && de->size<8)
   chunk=tetra_chunk;
  else if (de->format==float_format)
   chunk=octa_chunk;
  put_edit_mem(de);
  de->chunk=chunk;
  show_edit_mem(de);
  show_edit_windows(de);
  SetDlgItemText(de->hWnd, IDC_CHUNK, chunk_names[chunk]);
}





	
static void show_edit_name(dataedit *de)
{ if (de->insp==NULL)
	SetDlgItemText(de->hWnd,IDC_NAME,"Disconnected");
  else if (de->reg_name!=NULL)
	SetDlgItemText(de->hWnd,IDC_NAME,de->reg_name);
  else
  { char str[22]; /* big enough for the largest 8Byte integer */
    uint64_to_hex(de->address,str);
    SetDlgItemText(de->hWnd,IDC_NAME,str);
  }
}

/* the API for the data editor */

void de_update(HWND hDlg)
/* call after changes to the inspector or to the data */
{ 
  dataedit *de = (dataedit*)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
  if (de->insp==NULL || de->insp->de_offset<0)
  { de->reg_name=NULL;
	de->reg_offset=0;
	set_edit_format(de,hex_format);
	set_edit_chunk(de,byte_chunk);
	de->size=0;
	de->address=0;
  }
  else if (de->insp->regs!=NULL)
  {	struct register_def *r = &(de->insp->regs[de->insp->de_offset]);
    static char empty[1]={0};
	de->reg_name=r->name;
	if (de->reg_name==NULL) de->reg_name=empty;
  	de->size=r->size;
  	set_edit_format(de, r->format);
    set_edit_chunk(de,r->chunk);
	de->reg_offset=r->offset;
    if (de->size>0) de->insp->get_mem(r->offset,r->size,de->mem);
  }
  else
  {	de->reg_name=NULL;
  	de->address=de->insp->address+de->insp->de_offset;
    de->size=1<<de->insp->chunk;
  	set_edit_format(de, de->insp->format);
    set_edit_chunk(de, de->insp->chunk);
	de->reg_offset=0;
    if (de->size>0) de->insp->get_mem(de->insp->de_offset,de->size,de->mem);
  }
  show_edit_mem(de);
  show_edit_name(de);
  show_edit_windows(de);
}

void de_connect(HWND hDlg, inspector_def *insp)
/* call to associate a new inspector with the editor.
   call with NULL to disconnect the current editor */
{ dataedit *de = (dataedit*)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
  if (de==NULL) return;
  if (de->insp==insp) return;
  if (de->insp!=NULL) de_disconnect(de->insp);
  de->insp=insp;
  if (insp==NULL) de->size=0;
}

void de_save(HWND hDlg)
/* call to save the content of the editor back to to memory.
   This is the same as pressing the store button */
{ dataedit *de = (dataedit*)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
  if (de->insp!=NULL && de->insp->store !=NULL && de->insp->de_offset>=0)
  { put_edit_mem(de);
    if (de->reg_name!=NULL)
      de->insp->store(de->reg_offset,de->size,de->mem); 
	else
	  de->insp->store((int)(de->address-de->insp->address),de->size,de->mem); 
  }
}

static INT_PTR CALLBACK   
DataEditDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ static int min_w, min_h;
  switch ( message )
  { case WM_INITDIALOG :
      { RECT rect;
	    dataedit *de;
		register_subwindow(hDlg);
		de = new_dataedit();
		if (de==NULL) 
		{ DestroyWindow(hDlg);
		  return FALSE;
		}
	    SetWindowLongPtr(hDlg,DWLP_USER,(LONG)(LONG_PTR)de);
	    de->hWnd=hDlg;
	    SetDlgItemText(hDlg,IDC_FORMAT,format_names[de->format]);
	    SetDlgItemText(hDlg,IDC_CHUNK,chunk_names[de->chunk]);

		show_edit_mem(de);
		show_edit_windows(de);
	    SetFocus(GetDlgItem(hDlg,IDC_LOAD));
		GetClientRect(hDlg,&rect);
	    min_w = rect.right-rect.left;
	    min_h = rect.bottom-rect.top;
	    //get_font_metrics(hDlg);
	  }
	  return FALSE;
	case WM_CLOSE:
	  DestroyDataEdit(0);
	  return FALSE;
	case WM_DESTROY:
	  unregister_subwindow(hDlg);
	  free((dataedit*)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER));
      return FALSE;
	case DE_CONNECT:
	     de_connect(hDlg,(inspector_def *)lparam);
	  /* fall through to DE_UPDATE */
	case DE_UPDATE:
		de_update(hDlg);
	  return FALSE;

	case DE_SAVE:
	  de_save(hDlg);
	  return FALSE;

	case WM_COMMAND: 
      if (HIWORD(wparam) == BN_CLICKED) 
	  { dataedit *de = (dataedit*)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
		if (LOWORD(wparam) ==IDC_LOAD)   /* User has hit the Load key.*/
		{ if (de->insp!=NULL && de->insp->load!=NULL && de->insp->de_offset>=0) 
		  { if (de->reg_name!=NULL)
 			  memmove(de->mem,de->insp->load(de->reg_offset,de->size),de->size);
		    else
			  memmove(de->mem,de->insp->load((int)(de->address-de->insp->address),de->size),de->size);
		    show_edit_mem(de);
		  }
		}
		else if (LOWORD(wparam) ==IDC_STORE)   /* User has hit the store key.*/
		  de_save(hDlg);

	    else if (LOWORD(wparam) == IDC_FORMAT)
		   set_edit_format(de,de->format+1);
	    else if (LOWORD(wparam) == IDC_CHUNK)
		   set_edit_chunk(de,de->chunk+1);
	  }
	  return FALSE;
	case WM_SIZE:
		InvalidateRect(hDlg,NULL,TRUE);
		return FALSE;
	case WM_GETMINMAXINFO:
	{ MINMAXINFO *p = (MINMAXINFO *)lparam;
	  p->ptMinTrackSize.x = min_w;
      p->ptMinTrackSize.y = min_h;
	  p->ptMaxTrackSize.x=p->ptMinTrackSize.x;
	  p->ptMaxTrackSize.y=p->ptMinTrackSize.y;
	}
	return 0;

 }
  return FALSE;
}


HWND CreateDataEdit(HINSTANCE hInst, HWND hWnd)
/* call this to create an data editor child window. */
{ return CreateDialog(hInst,MAKEINTRESOURCE(IDD_DATAEDIT),hWnd,DataEditDialogProc);
}