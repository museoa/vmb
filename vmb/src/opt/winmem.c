#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "win32connect.h"
#include "winopt.h"
#include "option.h"
#include "param.h"
#include "error.h"
#include "bus-arith.h"
#include "winmem.h"


HWND hMemory=NULL; 
static uint64_t goto_addr=0x0;
static unsigned int mem_first=0, mem_size=0;
static unsigned char *mem_buf=NULL;
static unsigned int mem_range;
int (*mem_inspect)(unsigned int offset, int size, unsigned char *buf) = NULL;

/* scrolbar management */
static unsigned int sb_base=0;
static int sb_cur = 0, sb_rng=100;

static void sb_range(void)
/* determine and set range as virtual number of lines */
{ SCROLLINFO si;
  if (mem_range*10<vmb_size)
    sb_rng = mem_range*10;
  else if (mem_range<vmb_size)
    sb_rng = vmb_size;
  else
    sb_rng = mem_range;
		  

  if (mem_first<sb_base || mem_first>=(sb_base+sb_rng))
  { /* move base to goto addr*/
    sb_base = (unsigned int)(goto_addr-vmb_address);
    if (mem_first<sb_base) 
      sb_base = mem_first;
	else if (mem_first>sb_base+sb_rng)
	  sb_base =mem_first-sb_rng;
  }
  sb_cur= mem_first-sb_base;

  si.cbSize=sizeof(si);
  si.fMask=SIF_PAGE|SIF_POS|SIF_RANGE;
  si.nMin=0;
  si.nMax=sb_rng;
  si.nPage=mem_range;
  si.nPos=sb_cur;
  SetScrollInfo(GetDlgItem(hMemory,IDC_MEM_SCROLLBAR),SB_CTL,&si,TRUE);
}



static int adjust_goto_address(void)
/* 0 if no update needed, else return 1 */
{ int ret=0;
  if (goto_addr<vmb_address)
  { goto_addr=vmb_address;
    ret = 1;
  }
  else if (goto_addr>=vmb_address+vmb_size)
  { goto_addr=vmb_address+(vmb_size-1);
    ret=1;
  }
  return ret;
}

static RECT mem_rect;
static BOOL layout_change=TRUE;


void refresh_display()
{  InvalidateRect(hMemory,&mem_rect,layout_change);
   layout_change=FALSE;
}

static int adjust_mem_display(void)
/* 0 if no update needed, else return 1 */
{ int ret=0;
  static unsigned int old_size=0;
  mem_size = mem_range;
  if (mem_first+mem_size>vmb_size)
  { mem_size= vmb_size-mem_first;
    ret=1;
  }
  if (mem_size>old_size)
  { mem_buf=realloc(mem_buf,mem_size);
    if (mem_buf==NULL)
		vmb_fatal_error(__LINE__,"Out of memory");
    old_size=mem_size;
  }
  mem_inspect(mem_first,mem_size,mem_buf);
  sb_range();
  refresh_display();
  return ret;
}




enum {hex_format=0, unsigned_format=1, signed_format=2, ascii_format=3, float_format=4, double_format=5,last_format=5 } mem_format = hex_format;
char *format_names[]={"Hex","Dec","-Dec","Ascii","Float","Double"};

enum {byte_chunk=0, wyde_chunk=1,tetra_chunk=2,octa_chunk=3, last_chunk=3 } mem_chunk = byte_chunk;
static char *chunk_names[]={"BYTE","WYDE","TETRA","OCTA"};

static int mem_lines, mem_cols;
static int mem_width=0, mem_height=0;
static int line_height, digit_width, address_width, separator_width, separator_height, top_width; 

static void get_font_metrics(HWND hWnd)
{ SIZE size;
  HDC hDC=GetDC(hWnd);
  HFONT hfnt = GetStockObject(ANSI_FIXED_FONT); 
  SelectObject(hDC, hfnt);
  GetTextExtentPoint32(hDC,"0",1,&size);
  separator_height=2;
  digit_width=size.cx;
  line_height=size.cy+separator_height;
  separator_width = digit_width/2;
  GetTextExtentPoint32(hDC,"0x0000000000000000:",19,&size);
  address_width=size.cx;
  top_width=28;
  ReleaseDC(hWnd,hDC);
}

static int chunk_size=1;
static int lines, columns, column_width, column_digits, line_range;



static void resize_memory_dialog(void)
{ int sbw;
  RECT sbRect;

  GetWindowRect(GetDlgItem(hMemory,IDC_MEM_SCROLLBAR),&sbRect);
  sbw=sbRect.right-sbRect.left;
  MoveWindow(GetDlgItem(hMemory,IDC_MEM_SCROLLBAR),
	         mem_width-sbw,top_width,sbw,mem_height-top_width,TRUE);

  mem_rect.top=top_width;
  mem_rect.bottom=mem_height;
  mem_rect.left=0;
  mem_rect.right=mem_width-sbw;

  lines = (mem_height-top_width)/line_height;
  chunk_size=1<<mem_chunk;
  switch (mem_format)
  { case hex_format: column_digits=2*chunk_size; break;
    case ascii_format: column_digits=1*chunk_size; break;
	case signed_format:
      if (chunk_size==1) column_digits=4;
      else column_digits=1+5*chunk_size/2;
	  break;
	case unsigned_format:
      if (chunk_size==1) column_digits=3;
      else column_digits=5*chunk_size/2;
	  break;
	default:
	case float_format:
	case double_format:
       column_digits=19;
    break;
  }
  column_width=column_digits*digit_width+separator_width;
  columns = (mem_width-sbw-address_width-separator_width)/column_width;
  if (columns<1)
  { columns=1;
    column_width=mem_width-sbw-address_width-separator_width;
	if (column_digits*digit_width>column_width)
      column_digits=column_width/digit_width;
  }
  line_range = columns*chunk_size;
  mem_range= lines*line_range;
  layout_change=TRUE;
  adjust_mem_display();
}


/* Color management */

#define COLD RGB(0xFD,0xFD,0xFF)
#define HOT RGB(0xFF,0xA0,0xA0)
static unsigned char* old_mem=NULL;
static unsigned int old_size=0, old_first=0;

void update_old_mem(void)
{ if (old_size<mem_size)
  { old_mem = realloc(old_mem,mem_size);
    if (old_mem==NULL)
		vmb_fatal_error(__LINE__,"Out of memory");
  }
   /* newly displayed memory contente is cold, (copy from mem_buf)
      only previously displayed content that has changed is hot */
   if (old_first<mem_first)
   { /* move overlap of old and new down*/
      unsigned int dist = mem_first-old_first; 
      if (old_size>dist)
	  { int ovlap = old_size-dist;
	    memmove(old_mem, old_mem+dist,ovlap);
	    memmove(old_mem+ovlap,mem_buf+ovlap,mem_size-ovlap);
	  }
	  else
	    memmove(old_mem,mem_buf,mem_size);
   }
   else if (old_first>mem_first)
   { /* move overlap of old and new up*/
      unsigned int dist = old_first-mem_first; 
      if (old_size>dist)
	  { int ovlap = old_size-dist;
	    memmove(old_mem+dist, old_mem,ovlap);
	    memmove(old_mem,mem_buf,dist);
		if (mem_size>old_size)
	      memmove(old_mem+old_size,mem_buf+old_size,mem_size-old_size);
	  }
	  else
	    memmove(old_mem,mem_buf,mem_size);
   }
   else if (mem_size>old_size)
	 memmove(old_mem+old_size,mem_buf+old_size,mem_size-old_size);
   old_first=mem_first;
   old_size=mem_size;
}

static void refresh_old_mem(void)
{ if (old_size<mem_size)
  { old_mem = realloc(old_mem,mem_size);
    if (old_mem==NULL)
		vmb_fatal_error(__LINE__,"Out of memory");
  }
  memmove(old_mem,mem_buf,mem_size);
  old_first=mem_first;
  old_size=mem_size;
}

static int different(int offset, int size)
{ int i;
  for (i=0;i<size;i++)
  { if (old_mem[offset+i]!=mem_buf[offset+i]) return 1;
  }
  return 0;
}


static void chunk_to_str(char *str, unsigned char *buf)
{ int j;
  int w=column_digits;
  switch (mem_format)
  { default:
    case hex_format:
      for (j=0;j<chunk_size;j++)
	    sprintf(str+j*2,"%02X",buf[j]);
	  break;
    case ascii_format:
	  memmove(str,buf,chunk_size);
	  break;
	case unsigned_format:
      switch (chunk_size)
      { case 1: sprintf(str,"%*u",column_digits,buf[0]); break;
        case 2: sprintf(str,"%*u",column_digits,GET2(buf)); break;
        case 4: sprintf(str,"%*u",column_digits,GET4(buf)); break;
	    default:
        case 8: w = sprintf(str,"%*llu",column_digits,GET8(buf)); break;
      }
	  break;
	case signed_format:
      switch (chunk_size)
      { case 1: sprintf(str,"%*d",column_digits,(int8_t)buf[0]); break;
        case 2: sprintf(str,"%*d",column_digits,(int16_t)GET2(buf)); break;
        case 4: sprintf(str,"%*d",column_digits,(int32_t)GET4(buf)); break;
	    default:
        case 8: w=sprintf(str,"%*lld",column_digits,(int64_t)GET8(buf)); break;
      }
	  break;
	case float_format: 
	  w=sprintf(str,"%*g",column_digits,(float)GET4(buf)); 
	  break;
	case double_format:
	  w=sprintf(str,"%*g",column_digits,(double)GET4(buf)); 
	  break;
  }
  if (w!=column_digits)
	  memset(str,'*',column_digits);
}

void display_memory(HDC hdc)
{  uint64_t addr=vmb_address+mem_first;
   int i,k;
   char str[22]; /* big enough for the largest 8Byte integer */
   RECT r;

   SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
   SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
   for (i=0;i<lines && (unsigned int)(i*line_range)<mem_size;i++)
   { uint64tohex(addr+i*line_range,str);
	 str[18]=':';
	 TextOut(hdc,0,top_width+i*line_height,str,19);
   }
  update_old_mem();
  for (i=0;i<lines && (unsigned int)(i*line_range)<mem_size;i++)
   { int y;
     y=top_width+i*line_height;
	 for (k=0;k<columns && (unsigned int)((i*columns+k)*chunk_size)< mem_size;k++)
	 { int x;
       chunk_to_str(str, mem_buf+(i*columns+k)*chunk_size);
       if (different((i*columns+k)*chunk_size,chunk_size))
	     SetBkColor(hdc,HOT);
	   else
	     SetBkColor(hdc,COLD);
	   x = address_width+separator_width+k*column_width;
	   SetTextAlign(hdc,TA_RIGHT|TA_NOUPDATECP);
	   r.top=y;
       r.left=x;
       r.right=x+column_width-separator_width;
       r.bottom=y+line_height-separator_height;

       ExtTextOut(hdc,x+column_width-separator_width,y,
		   ETO_OPAQUE,&r,str,column_digits,NULL);
	 }
   } 
}



INT_PTR CALLBACK   
MemoryDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ switch ( message )
  { case WM_INITDIALOG :
      adjust_goto_address();
      uint64tohex(goto_addr,tmp_option);
      SetDlgItemText(hDlg,IDC_GOTO,tmp_option);
	  SetDlgItemText(hDlg,IDC_FORMAT,format_names[mem_format]);
	  SetDlgItemText(hDlg,IDC_CHUNK,chunk_names[mem_chunk]);
	  SetFocus(GetDlgItem(hDlg,IDOK));
	  get_font_metrics(hDlg);
      /*SendMessage(hDlg,DM_SETDEFID,(WPARAM)IDOK,0); */
	  return FALSE;	
	case WM_COMMAND: 
	  if (wparam ==IDOK)
	  {  /* User has hit the ENTER key.*/
		 if (GetFocus()==GetDlgItem(hDlg,IDC_GOTO))
        { GetDlgItemText(hDlg,IDC_GOTO,tmp_option,MAXTMPOPTION);
    	  goto_addr = strtouint64(tmp_option);
		  if (adjust_goto_address())
		  { uint64tohex(goto_addr,tmp_option);
            SetDlgItemText(hDlg,IDC_GOTO,tmp_option);
		  }
		  mem_first = (unsigned int)(goto_addr-vmb_address);
		  adjust_mem_display();
		 }
	  }
	  else if (HIWORD(wparam) == BN_CLICKED) 
	  { if ((HWND)lparam==GetDlgItem(hDlg,IDC_FORMAT))
	    { mem_format++; if (mem_format>last_format) mem_format=0;
		  SetDlgItemText(hDlg, IDC_FORMAT, format_names[mem_format]);
		  if (mem_format==float_format)
		  { mem_chunk=tetra_chunk;
            SetDlgItemText(hDlg, IDC_CHUNK, chunk_names[mem_chunk]);
		  } 
		  else if (mem_format==double_format)
		  { mem_chunk=octa_chunk;
            SetDlgItemText(hDlg, IDC_CHUNK, chunk_names[mem_chunk]);
		  } 
     	  resize_memory_dialog();
	    } 
	    else if ((HWND)lparam==GetDlgItem(hDlg,IDC_CHUNK))
	    { if (mem_format==float_format)
		    mem_chunk=tetra_chunk;
		  else if (mem_format==double_format)
		    mem_chunk=octa_chunk;
		  else
		    mem_chunk++; 
		  if (mem_chunk>last_chunk) mem_chunk=0;
		  SetDlgItemText(hDlg, IDC_CHUNK, chunk_names[mem_chunk]);
     	  resize_memory_dialog();
	    }
	  }
	  return FALSE;
	case WM_VSCROLL: 
      switch (LOWORD(wparam)) 
	  { case SB_LINEUP:
	      if (mem_first > (unsigned int)(line_range))
	        mem_first =mem_first-line_range;
		  else
			mem_first=0;
		  break;
        case SB_LINEDOWN: 
	      if (mem_first+mem_size+line_range<vmb_size)
	        mem_first =mem_first+line_range;
		  else
			mem_first=vmb_size-mem_range;
		  break;
		case SB_PAGEUP:
	      if (mem_first > (unsigned int)mem_range)
	        mem_first =mem_first-mem_range;
		  else
			mem_first=0;
		  break;
        case SB_PAGEDOWN: 
	      if (mem_first+mem_size+mem_range<vmb_size)
	        mem_first =mem_first+mem_range;
		  else
			mem_first=vmb_size-mem_range;
		  break;
		case SB_TOP:
			mem_first=0;
		  break;
        case SB_BOTTOM: 
			mem_first=vmb_size-mem_range;
		  break;
		case SB_THUMBTRACK: /* HIWORD(wparam) is 0 to sb_range */
		     mem_first=sb_base+(HIWORD(wparam)/line_range)*line_range;
          break;
		default:
		case SB_ENDSCROLL:
		case SB_THUMBPOSITION:
		  return FALSE;
	  }
	   adjust_mem_display();
       return TRUE; 
    case SBM_SETRANGE:
		return TRUE;
	case WM_PAINT:
    { PAINTSTRUCT ps;
	  HDC hdc = BeginPaint (hDlg, &ps);
      display_memory(hdc);
      EndPaint (hDlg, &ps);
	  return TRUE;
    }    
	case WM_SIZE:
		mem_width=LOWORD(lparam);
		mem_height=HIWORD(lparam);
		resize_memory_dialog();
	  break;
  }
  return FALSE;
}



void mem_update(unsigned int offset, int size)
{ if (hMemory==NULL) return;
  adjust_goto_address();
  if (offset>=mem_first+mem_size || offset+size<=mem_first) 
    return;
  else
  { unsigned int from, to;
    refresh_old_mem();
    if (offset<mem_first) from=mem_first; else from=offset;
	if (offset+size<mem_first+mem_size) to = offset+size; else to = mem_first+mem_size;
	mem_inspect(from, to-from, mem_buf+(from-mem_first));
	refresh_display();
  }
}
