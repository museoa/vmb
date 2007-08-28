/*
  Copyright 2005 Anton Wolf, Martin Ruckert
    
  ruckertm@acm.org

  This file is part of the MMIX Motherboard project

  This file is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this software; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <stdlib.h>
#include <string.h>
#ifdef WIN32

#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include "resource.h"



/* Windows needs a different main program (taken from win32main.c) */

HWND hMainWnd,hpower,hDebug=NULL;

HBITMAP hon,hoff,hconnect;

#define WM_SOCKET (WM_USER+1)


#else
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "errno.h"
#endif
#include "option.h"
#include "param.h"
#include "error.h"
#include "message.h"
#include "bus-arith.h"


char version[]="$Revision: 1.1 $ $Date: 2007-08-28 10:12:06 $";

char howto[] =
"\n"

"The program first reads the configuration file, \"default.mmc\".\n"
"Then, the program waits for other simulated hardware modules\n"

"to connect. It recieves and redirects messages\n"

"from and to these modules.\n"
#ifndef WIN32

"Further it executes the following commands\n"
"\n"
"help\t\tdisplay this help message\n"
"debug\t\tswitch on debugging output\n"
"nodebug\t\tswitch off debugging output\n"
"quit\t\tterminte all slots and the motherboard\n"
"on\t\tsend all slots the power on message\n"
"off\t\tsend all slots the power off message\n"
"reset\t\tsend all slots the reset message\n"
"info\t\tdisplay information about the slots in use\n"
"close n\t\tclose slot number n\n"
"slot n\t\tdisplay details about slot n\n"
"signal n\t\tsend signal n to the devices\n"
"\n"

#endif
;


#define SLOTS 255

static struct
{
  int fd;/* File Descriptor */

  unsigned char *name;
  unsigned char from_addr[8];/* Address from */
  unsigned char to_addr[8]; /* Address to */
  unsigned int hi_mask,low_mask; /* Enabled for interrupts */
  struct sockaddr addr; /* Network Address */
  int address_size; /* size of addr */
} slot[SLOTS];


static int exitflag = 0;
static int powerflag = 0;
static int read_ops = 0;
static int write_ops = 0;


static int max_slot=0;  /* Strict upper Bound on Slot Numbers */
static int mother_fd= INVALID_SOCKET;   /* Server File Descriptor */
static int max_fd = 0;  /* Biggest File Descriptor */
static fd_set read_fdset, write_fdset;
static struct sockaddr_in mother_addr; /* Server Network Address */

/*Message*/
static  unsigned char mtype;
static  unsigned char msize;
static  unsigned char mslot;
static  unsigned char mid;
static  unsigned int  mtime;
static  unsigned char maddress[8];
static  unsigned char mpayload[MAXPAYLOAD];


void initialize_slots()
{
  int i;
  memset(slot,0,sizeof(slot));
       
  for(i=0; i<SLOTS; i++)
    { slot[i].fd = INVALID_SOCKET;
    }
  max_fd = mother_fd;
  max_slot = 0;
}

void create_server()
{
  mother_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (!valid_socket(mother_fd))
    fatal_error(__LINE__, "Can't create new server socket");



  /* make the socket non blocking */
#ifdef WIN32
  if (hMainWnd)
    WSAAsyncSelect(mother_fd, hMainWnd, WM_SOCKET, FD_READ | FD_CONNECT |FD_ACCEPT | FD_CLOSE)
  else
  {	 WSAEVENT  e;
	 e = WSACreateEvent();
	 WSAEventSelect (mother_fd, e,FD_READ | FD_CONNECT | FD_ACCEPT | FD_CLOSE);
  }
#else
  { int flags;
    flags = fcntl(mother_fd,  F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(mother_fd, F_SETFL,flags);
  }
#endif


  /* Allow rapid reuse of this port. */
  {
    int tmp = 1;
    setsockopt (mother_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp, sizeof (tmp));
  }
  max_fd = mother_fd;
	
  mother_addr.sin_family = AF_INET;
  mother_addr.sin_port = htons((unsigned short)port);
  mother_addr.sin_addr.s_addr = INADDR_ANY;

  if((bind(mother_fd,(struct sockaddr *) &mother_addr, sizeof(mother_addr))) != 0)
  { bus_disconnect(mother_fd);
    fatal_error(__LINE__, "Can't bind server socket to address");
  }
	
  if(listen(mother_fd, 5) != 0)
  { bus_disconnect(mother_fd);
    fatal_error(__LINE__, "Can't listen");
  }
  debugi("Created server at Port %d", port);
}

int write_to_slot(int i)
{ if (send_msg(slot[i].fd, 1,mtype, msize,mslot,mid,mtime,maddress,mpayload)<=0)
  { errormsg("Unable to deliver message");
    return 1;
  }
  debugx("Send type:    %s",&mtype,1);
  debugi("Send size:    %d",msize);
  debugi("Send slot:    %d",mslot);

  debugx("Send id:      %s",&mid,1);
  if (mtype&TYPE_TIME)
    debugi("Send time:    %d",mtime);
  if (mtype&TYPE_ADDRESS)
    debugx("Send address: %s",maddress,8);
  if (mtype&TYPE_PAYLOAD)
    debugx("Send payload: %s ...",mpayload,8*(msize+1));
  write_ops++;
  return 0;
}
 

static int bus_msg(unsigned char id, int i)
{
  mtype=TYPE_BUS;
  msize=0;
  mslot=0;
  mid=id;
  return write_to_slot(i);
}

static void power_on(int i)
{
  if (!bus_msg(ID_POWERON,i))
    debugi("Sent Power On to Slot %d", i);
  else
   errormsg("Unable to send Power On");
}

static void power_off(int i)
{ if (!bus_msg(ID_POWEROFF,i))
    debugi("Sent Power Off to Slot %d", i);
  else
   errormsg("Unable to send Power Off");
}

static void reset(int i)
{ 
  if (!bus_msg(ID_RESET,i))
    debugi("Sent Reset to Slot %d", i);
  else 
   errormsg("Unable to send Reset");
}

void shutdown_device(int i)
{ 
  if(!bus_unregister(slot[i].fd))
   debugi("Shutdown of Slot %d", i);
 else 
   errormsg("Unable to shut down slot");
}

void remove_slot(int slotnr)
{ slot[slotnr].fd = INVALID_SOCKET;
  if (slot[slotnr].name!=NULL)
  { free(slot[slotnr].name);
    slot[slotnr].name=NULL;
  }
  while (max_slot>0 && !valid_socket(slot[max_slot-1].fd)) max_slot--;
  debugi("Removed Slot %d", slotnr);
}


void close_device(int slotnr)
{  if (bus_disconnect(slot[slotnr].fd)>=0)
       debugi("Closed socket from Slot %d : Successful", slotnr);
   else
       debugi("Closed socket from Slot %d : Failed", slotnr);	
   remove_slot(slotnr);
   debugi("Disconnected device at Slot %d", slotnr);
}

void disconnect_device(int slotnr)
{  if (powerflag)  power_off(slotnr);
   shutdown_device(slotnr);
   close_device(slotnr);
}


int find_slot(unsigned char address[8])
{ int i;
  for(i = 0; i < max_slot; i++)
    if(valid_socket(slot[i].fd) &&
       in_range(slot[i].from_addr, address, slot[i].to_addr)) 
     return i;
  return -1;
}
  
void for_all_slots(void proc(int i))
{ int i;
  for(i = 0; i < max_slot; i++)
   if(valid_socket(slot[i].fd))
	  proc(i);
}

void shutdown_server()
{
  for_all_slots(disconnect_device);
  if (bus_unregister(mother_fd)>=0 &&
      bus_disconnect(mother_fd)>= 0)		
	debugi("Shutdown server at Port %d : Successful", port);
}

void add_to_read_fdset(int i)
{  FD_SET(slot[i].fd, &read_fdset);
}

void build_read_fdset()
{ FD_ZERO(&read_fdset);
  FD_SET(0, &read_fdset);
  FD_SET(mother_fd, &read_fdset);
  for_all_slots(add_to_read_fdset);
}


void send_interrupt_to(int i)
{ if(((mslot < 32) && (slot[i].low_mask & BIT(mslot))) || 
     ((mslot >= 32) && (slot[i].hi_mask & BIT(mslot-32))))
     {
       if(!write_to_slot(i))
         debugi("Sent Interrupt to Slot %d", i);
       else 
         errormsg("Unable to send Interrupt");
     }
}

void interrupt_all(int i)
{ mtype=TYPE_BUS;
  msize=0;
  mslot=i;
  mid=ID_INTERRUPT;
  debugi("Sending Interrupt %d", mslot);
  for_all_slots(send_interrupt_to);
}


void power_all(int state)
{ if (powerflag == state)
    return;
  powerflag = state;
  debugs("All slots  power %s", state?"on":"off");

  if (powerflag) for_all_slots(power_on); 
  else for_all_slots(power_off); 

#ifdef WIN32
  if (powerflag)
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon); 
  else
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
#endif
}

void reset_all(void)
{ for_all_slots(reset);
  debug("All slots reset");
}

void send_dummy_answer(int dest_slot)
{
  if(valid_socket(slot[dest_slot].fd))
    {
      mtype=TYPE_NULL;
      msize=0;
      mslot=0;
      mid=ID_NOREPLY;
      if (!write_to_slot(dest_slot))
	debugi("Sent dummy answer to Slot %d", dest_slot);
      else 
	errormsg("Unable to send dummy answer");
   }
}

void interpret_message(int source_slot)
{ debugx("Received type:    %s",&mtype,1);
  debugi("Received size:    %d",msize);
  debugi("Received slot:    %d",mslot);
  debugx("Received id:      %s",&mid,1);
  if (mtype&TYPE_TIME)
    debugi("Received time:    %d",mtime);
  if (mtype&TYPE_ADDRESS)
    debugx("Received address: %s",maddress,8);
  if (mtype&TYPE_PAYLOAD)
    debugx("Received payload: %s ...",mpayload,8*(msize+1));

  if(mtype & TYPE_BUS)
    {
      switch(mid)
      {	case(ID_IGNORE):
	  break;
	case(ID_INTERRUPT):
	  interrupt_all(mslot);
	  break;
	case(ID_REGISTER):
	   memmove(slot[source_slot].from_addr, &mpayload[0], 8);
           memmove(slot[source_slot].to_addr, &mpayload[8], 8);
           slot[source_slot].hi_mask = chartoint(&mpayload[16]);
           slot[source_slot].low_mask = chartoint(&mpayload[20]);
	   if (msize > 2) 
           { int n;
	     n = strlen(mpayload+24)+1;
             slot[source_slot].name = malloc(n);
             if (slot[source_slot].name==NULL)
               errormsg("Out of memory");
	     else
	       strcpy(slot[source_slot].name,mpayload+24);
	   }
	   if (powerflag) power_on(source_slot);
	   break;
	case(ID_POWERON):
	   if (!powerflag)  power_on(source_slot);
	   break;
	default:
	   break;
      }
    }
  else if(mtype & TYPE_ROUTE)
    {
      int dest_slot = mslot;
      if(mslot >= max_slot)
	  {
	    debugi("Slot number %d not available", mslot);
	    if(mtype & TYPE_REQUEST)
	      send_dummy_answer(source_slot);
	    return;
	  }
      if(mtype & TYPE_REQUEST)
	mslot = source_slot;
      if (write_to_slot(dest_slot) && (mtype & TYPE_REQUEST))
	send_dummy_answer(source_slot);
    }
  else if(mtype & TYPE_ADDRESS)
    {
      int dest_slot = find_slot(maddress);
      if(dest_slot < 0)
	{
	  debugx("Address #%s not available",maddress,8); 
	  if(mtype & TYPE_REQUEST)
	    send_dummy_answer(source_slot);
	  //raise interrupt for non existing memory
	  interrupt_all(INT_NOMEM);
	}
      else
	{

	  if(mtype & TYPE_REQUEST)
	    mslot = source_slot;

	  if (write_to_slot(dest_slot) &&(mtype & TYPE_REQUEST))
	    send_dummy_answer(source_slot);
	}
    }
}


void close_slot(int i)
{
  if (i >= 0 && i < SLOTS && valid_socket(slot[i].fd))
    disconnect_device(i);
  else
    errormsg("Invalid slot number");
}


void connect_new_device()
{ int i;
  for(i = 0; i < SLOTS; i++)
    if(!valid_socket(slot[i].fd))
    { 
        slot[i].address_size=sizeof(slot[i].addr);
        slot[i].fd = accept(mother_fd, &(slot[i].addr), &(slot[i].address_size));
        if (!valid_socket(slot[i].fd))
          return;
        if(i >= max_slot) max_slot=i+1;
        if(slot[i].fd > max_fd)   max_fd = slot[i].fd;
        debugi("Connected device at Slot %d", i);
        { /* Tell TCP not to delay small packets.  This greatly speeds up
      	       interactive response. */
          int tmp = 1;
          setsockopt (slot[i].fd, IPPROTO_TCP, TCP_NODELAY,
                      (char *) &tmp, sizeof (tmp));
        }
        return;
     }
 errormsg("Can't connect any more client's");
}

#ifdef WIN32
void close_socket(int fd)
{ int i;
  for (i=0;i<max_slot;i++)
    if(valid_socket(slot[i].fd) && slot[i].fd==fd)
    {  close_slot(i);
	   return;
    }
}
#endif

#ifdef WIN32


void process_read_fdset(int fd)
{ int i;
  for (i=0;i<max_slot;i++)
  {
    if(valid_socket(slot[i].fd) && slot[i].fd==fd)
    { 
      int error;
      error = receive_msg(slot[i].fd, 0, &mtype, &msize, &mslot, &mid, &mtime, maddress, mpayload);
      if (error < 0 )
	remove_slot(i);
      else if (error > 0)
	interpret_message(i);
    }
  }
}
#else

void process_read_fdset()
{
  static int i=0;
  int k;
  for (k=0;k<max_slot;k++)
  { i=(i+1)%(max_slot);
    if(valid_socket(slot[i].fd) && FD_ISSET(slot[i].fd, &read_fdset))
    { 
      int error;
      error = receive_msg(slot[i].fd, 0, &mtype, &msize, &mslot, &mid, &mtime, maddress, mpayload);
      if (error < 0 )
	remove_slot(i);
      else if (error > 0)
	interpret_message(i);
    }
  }

}

#endif






#ifdef WIN32


#define MAX_LOADSTRING 100

/* Global Variables: */
HINSTANCE hInst;					
TCHAR szClassName[MAX_LOADSTRING] ="WIN32HARDWARE";
TCHAR szTitle[MAX_LOADSTRING] ="WIN32HARDWARE";
HBITMAP hBmp;
HMENU hMenu;


static int infoslot = -1;


void display_slot(HWND hDlg,int i)

{  if(valid_socket(slot[i].fd))

	{ char hex[17]={0};

      char buffer[4];

      int size;

	  infoslot = i;

	  if (slot[i].name !=NULL)

        SetDlgItemText(hDlg,IDC_SLOTNAME,slot[i].name); 

	  else

       SetDlgItemText(hDlg,IDC_SLOTNAME,"Unnamed"); 



      chartohex(slot[i].from_addr,hex,8);

      SetDlgItemText(hDlg,IDC_SLOTSTART,hex);            



      chartohex(slot[i].to_addr,hex,8);

      SetDlgItemText(hDlg,IDC_SLOTLAST,hex); 



      size = get_offset(slot[i].from_addr,slot[i].to_addr);

      inttochar(hi_offset,buffer);

      chartohex(buffer,hex,4);

      inttochar(size,buffer);

      chartohex(buffer,hex+8,4);

      SetDlgItemText(hDlg,IDC_SLOTSIZE,hex); 



      inttochar(slot[i].hi_mask,buffer);

      chartohex(buffer,hex,4);

      inttochar(slot[i].low_mask,buffer);

      chartohex(buffer,hex+8,4);

	  SetDlgItemText(hDlg,IDC_SLOTMASK,hex); 

	}

	else

	{ 

      SetDlgItemText(hDlg,IDC_SLOTNR,"Not connected");            

      SetDlgItemText(hDlg,IDC_SLOTSTART,"");            

      SetDlgItemText(hDlg,IDC_SLOTLAST,""); 

	  SetDlgItemText(hDlg,IDC_SLOTSIZE,""); 

	  SetDlgItemText(hDlg,IDC_SLOTMASK,""); 

	}		

}


BOOL APIENTRY   
InfoDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 

  switch ( message )
    { case WM_INITDIALOG:
		{ TCITEM tie; 

		  char label[10];

		  int i;
          tie.mask = TCIF_TEXT; 
          tie.iImage = -1; 

		  for(i = 0; i < max_slot; i++)

			{ sprintf(label,"Slot %d",i);

              tie.pszText = label; 
              TabCtrl_InsertItem(GetDlgItem(hDlg,IDC_SLOTS), i, &tie);

			}

          if (max_slot>0)

		  {  TabCtrl_SetCurSel(GetDlgItem(hDlg,IDC_SLOTS), 0);

		     display_slot(hDlg,0);

		  }  
	       return TRUE;
		}
    case WM_NOTIFY: 
          /*  if (HIWORD(wparam)==TCN_SELCHANGE) */ { 
              int i = TabCtrl_GetCurSel(GetDlgItem(hDlg,IDC_SLOTS)); 

			  display_slot(hDlg,i);
          } 
          break;       

    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
	{ EndDialog(hDlg, TRUE);

	  infoslot = -1;
        return TRUE;
	}
      break;
    case WM_COMMAND:
      if( wparam == IDC_SLOTCLOSE && infoslot >=0 && valid_socket(slot[infoslot].fd))

	  { close_slot(infoslot);

	      display_slot(hDlg,infoslot);

	  }

      else if (wparam == IDC_SLOTRESET && infoslot >=0 && valid_socket(slot[infoslot].fd))

		reset(infoslot);
      else if (wparam == IDOK)
	  { EndDialog(hDlg, TRUE);

	  	infoslot = -1;
        return TRUE;
	  }
      break;
    }
  return FALSE;
}


BOOL APIENTRY   
DebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
    { case WM_SYSCOMMAND:
	if( wparam == SC_CLOSE ) 
	  { hDebug=NULL;
	  EndDialog(hDlg, TRUE);
	  return TRUE;
	  }
	break;
    case WM_SIZE: 
      MoveWindow(GetDlgItem(hDebug,IDC_DEBUG),5,5,LOWORD(lparam)-10,HIWORD(lparam)-10,TRUE); 
      return TRUE;
      break;
    }
  return FALSE;
}


BOOL APIENTRY   
AboutDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
    { case WM_INITDIALOG:

		SetDlgItemText(hDlg,IDC_VERSION,version);

		SetDlgItemText(hDlg,IDC_HOWTO,howto);


	return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
	{ EndDialog(hDlg, TRUE);
        return TRUE;
	}
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
	{
	  EndDialog(hDlg, TRUE);
	  return TRUE;
	}
      break;
    }
  return FALSE;
}


BOOL APIENTRY   

OnOffDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )

{

  switch ( message )

    { case WM_INITDIALOG:

    	return TRUE;

      case WM_COMMAND:

        if( HIWORD(wparam) == BN_CLICKED)

		{ power_all(!powerflag);

	      return TRUE;

		}

        break;

    }

  return FALSE;

}
BOOL APIENTRY   

ResetDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )

{

  switch ( message )

    { case WM_INITDIALOG:

    	return TRUE;

      case WM_COMMAND:

		if( HIWORD(wparam) == BN_CLICKED)

		{ reset_all();

	      return TRUE;

		}



      break;

    }

  return FALSE;

}







LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {   
  case WM_NCHITTEST:


    return HTCAPTION;
		    
  case WM_CREATE: 
	{ HWND h;

	  h = CreateDialog(hInst,MAKEINTRESOURCE(IDD_ONOFF),hWnd,OnOffDialogProc);

      SetWindowPos(h,HWND_TOP,25,222,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);

	  h = CreateDialog(hInst,MAKEINTRESOURCE(IDD_RESET),hWnd,ResetDialogProc);

      SetWindowPos(h,HWND_TOP,25,260,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);

	}

    hpower = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,

                          20,300,32,32,hWnd,NULL,hInst,0);

    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);


    return 0;
  case WM_SOCKET:
    { int error = WSAGETSELECTERROR(lParam);
      int event = WSAGETSELECTEVENT(lParam);

	  debugi("Socket event %d",event);

	  debugi("Socket error %d",error);
      if (event == FD_CLOSE || error != 0)
      {	close_socket(wParam);
      }
      else if (event == FD_READ)
        process_read_fdset(wParam);
      else if (event == FD_WRITE)
      ;
      else if (event == FD_CONNECT)
      {
      }
      else if (event == FD_ACCEPT)

		connect_new_device();
    }
    return 0;
  case WM_PAINT:
    {	PAINTSTRUCT ps;
    HDC hdc;

    hdc = BeginPaint (hWnd, &ps);
    if (hBmp)
      {	HDC memdc = CreateCompatibleDC(NULL);
      HBITMAP h = (HBITMAP)SelectObject(memdc, hBmp);
      BITMAP bm;
      GetObject(hBmp, sizeof(bm), &bm);

      BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, memdc, 0, 0, SRCCOPY);
      SelectObject(memdc, h);
      }
    EndPaint (hWnd, &ps);
    }
    return 0;
  case WM_NCRBUTTONDOWN:
    TrackPopupMenu(GetSubMenu(hMenu,0),TPM_LEFTALIGN|TPM_TOPALIGN,
		   LOWORD(lParam),HIWORD(lParam),0 ,hWnd,NULL);
    return 0;
  case WM_COMMAND:
    if (HIWORD(wParam)==0) /* Menu */
      switch(LOWORD(wParam))
	{ case ID_EXIT:
	    PostQuitMessage(0);
	    return 0;

	case ID_INFO:
          DialogBox(hInst,MAKEINTRESOURCE(IDD_INFO),hMainWnd,InfoDialogProc);
	  return 0; 
	case ID_DEBUG:
	  if (hDebug==NULL)
	    hDebug= CreateDialog(hInst,MAKEINTRESOURCE(IDD_DEBUG),hWnd,DebugDialogProc);
	  return 0; 
	case ID_HELP_ABOUT:
	  DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,AboutDialogProc);
	  return 0; 
	}
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  default:
    return (DefWindowProc(hWnd, message, wParam, lParam));
  }
 return (DefWindowProc(hWnd, message, wParam, lParam));
}



/* 	BitmapToRegion :  Create a region from the "non-transparent" pixels of a bitmap
 	Author :	  Jean-Edouard Lachand-Robert 
                          (http://www.geocities.com/Paris/LeftBank/1160/resume.htm), 
                          June 1998. 
 	hBmp :	          Source bitmap 
 	A pixel is assumed to be transparent if the value of each of its 
        3 components (blue, green and red) is  
 	equal to the corresponding value in the top left pixel.
*/

HRGN BitmapToRegion (HBITMAP hBmp)
{
  HRGN hRgn = NULL;

  if (hBmp)
    {
      // Create a memory DC inside which we will scan the bitmap content
      HDC hMemDC = CreateCompatibleDC(NULL);
      if (hMemDC)
	{
	  // Get bitmap size
	  BITMAP bm;
	  BITMAPINFOHEADER RGB32BITSBITMAPINFO;
	  VOID * pbits32; 
	  HBITMAP hbm32; 
			
	  GetObject(hBmp, sizeof(bm), &bm);

	  ZeroMemory(&RGB32BITSBITMAPINFO, sizeof(RGB32BITSBITMAPINFO));

	  RGB32BITSBITMAPINFO.biSize=sizeof(BITMAPINFOHEADER);
	  RGB32BITSBITMAPINFO.biWidth=		bm.bmWidth;
	  RGB32BITSBITMAPINFO.biHeight=		bm.bmHeight;
	  RGB32BITSBITMAPINFO.biPlanes=		1;
	  RGB32BITSBITMAPINFO.biBitCount=		32;

	  RGB32BITSBITMAPINFO.biCompression=  BI_RGB;

	  hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
		
	  if (hbm32)
	    {
	      HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

	      // Create a DC just to copy the bitmap into the memory DC
	      HDC hDC = CreateCompatibleDC(hMemDC);
	      if (hDC)
		{
		  // Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits)
		  BITMAP bm32;
		  HBITMAP holdBmp;
		  DWORD maxRects;
		  HANDLE hData; 
		  RGNDATA *pData;
		  BYTE *p32,lr,lg,lb;
		  HRGN h;
		  int x,y;
		  GetObject(hbm32, sizeof(bm32), &bm32);
		  while (bm32.bmWidthBytes % 4)
		    bm32.bmWidthBytes++;

		  // Copy the bitmap into the memory DC
		  holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
		  BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

		  // For better performances, we will use the ExtCreateRegion() function to create the
		  // region. This function take a RGNDATA structure on entry. We will add rectangles by
		  // amount of ALLOC_UNIT number in this structure.
#define ALLOC_UNIT	100
		  maxRects = ALLOC_UNIT;
		  hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
		  pData = (RGNDATA *)GlobalLock(hData);
		  pData->rdh.dwSize = sizeof(RGNDATAHEADER);
		  pData->rdh.iType = RDH_RECTANGLES;
		  pData->rdh.nCount = pData->rdh.nRgnSize = 0;
		  SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);


		  // Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
		  p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;

		  // Keep on hand highest and lowest values for the "transparent" pixels
		  lr = GetRValue(*p32);
		  lg = GetGValue(*p32);
		  lb = GetBValue(*p32);

		  for (y = 0; y < bm.bmHeight; y++)
		    {
		      // Scan each bitmap pixel from left to right
		      for (x = 0; x < bm.bmWidth; x++)
			{
			  // Search for a continuous range of "non transparent pixels"
			  int x0 = x;
			  LONG *p = (LONG *)p32 + x;
			  while (x < bm.bmWidth)
			    {
			      BYTE b = GetRValue(*p);
			      if (b == lr)
				{
				  b = GetGValue(*p);
				  if (b == lg )
				    {
				      b = GetBValue(*p);
				      if (b == lb )
					// This pixel is "transparent"
					break;
				    }
				}
			      p++;
			      x++;
			    }

			  if (x > x0)
			    {   RECT *pr;
			    // Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
			    if (pData->rdh.nCount >= maxRects)
			      {
				GlobalUnlock(hData);
				maxRects += ALLOC_UNIT;
				hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
				pData = (RGNDATA *)GlobalLock(hData);
			      }
			    pr = (RECT *)&pData->Buffer;
			    SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
			    if (x0 < pData->rdh.rcBound.left)
			      pData->rdh.rcBound.left = x0;
			    if (y < pData->rdh.rcBound.top)
			      pData->rdh.rcBound.top = y;
			    if (x > pData->rdh.rcBound.right)
			      pData->rdh.rcBound.right = x;
			    if (y+1 > pData->rdh.rcBound.bottom)
			      pData->rdh.rcBound.bottom = y+1;
			    pData->rdh.nCount++;

			    // On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
			    // large (ie: > 4000). Therefore, we have to create the region by multiple steps.
			    if (pData->rdh.nCount == 2000)
			      {
				HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
				if (hRgn)
				  {
				    CombineRgn(hRgn, hRgn, h, RGN_OR);
				    DeleteObject(h);
				  }
				else
				  hRgn = h;
				pData->rdh.nCount = 0;
				SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
			      }
			    }
			}

		      // Go to next row (remember, the bitmap is inverted vertically)
		      p32 -= bm32.bmWidthBytes;
		    }

		  // Create or extend the region with the remaining rectangles
		  h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
		  if (hRgn)
		    {
		      CombineRgn(hRgn, hRgn, h, RGN_OR);
		      DeleteObject(h);
		    }
		  else
		    hRgn = h;

		  // Clean up
		  SelectObject(hDC, holdBmp);
		  DeleteDC(hDC);
		}

	      DeleteObject(SelectObject(hMemDC, holdBmp));
	    }

	  DeleteDC(hMemDC);
	}	
    }

  return hRgn;
}

BOOL InitInstance(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;
  BITMAP bm;

  hInst = hInstance; 
  ZeroMemory(&wcex, sizeof(wcex));
  wcex.cbSize = sizeof(WNDCLASSEX); 
  wcex.style			= CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc	= (WNDPROC)WndProc;
  wcex.cbClsExtra		= 0;
  wcex.cbWndExtra		= 0;
  wcex.hInstance		= hInstance;
  wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
  wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
  /*	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
   */	
  wcex.lpszMenuName	= NULL;
  wcex.lpszClassName = szClassName;
  wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
  if (!RegisterClassEx(&wcex)) return FALSE;

  GetObject(hBmp, sizeof(bm), &bm);

  hMainWnd = CreateWindow(szClassName, szTitle ,WS_POPUP,
			  0, 0, bm.bmWidth, bm.bmHeight,
			  NULL, NULL, hInstance, NULL);

  if (hMainWnd) 
    { 
      HRGN h = BitmapToRegion(hBmp);
      if (h) SetWindowRgn(hMainWnd, h, TRUE);
    }

  return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
  HACCEL hAccelTable;
  MSG msg;
  WSADATA wsadata;
  LoadString(hInstance, IDS_CLASS, szClassName, MAX_LOADSTRING);
  LoadString(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);
  hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));
  hBmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BITMAP), 
			    IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  hon = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_ON), 
			   IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
  hconnect = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_CONNECT), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
  hoff = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OFF), 
			    IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
  if (hBmp==NULL) return FALSE;
  if(WSAStartup(MAKEWORD(1,1), &wsadata) != 0)
  {  errormsg("Unable to initialize Winsock dll");
     return FALSE;
  }
  
  InitCommonControls(); 

  if (!InitInstance (hInstance)) return FALSE;
  param_init();
  hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
  ShowWindow(hMainWnd, nCmdShow);
  UpdateWindow(hMainWnd);

  if (debugflag)
    SendMessage(hMainWnd,WM_COMMAND,(WPARAM)ID_DEBUG,0);
  initialize_slots();
  create_server();
  do_commands();

  while (GetMessage(&msg, NULL, 0, 0)) 
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
      { TranslateMessage(&msg);
      DispatchMessage(&msg);
      }
  shutdown_server();
  WSACleanup();
  return (msg.wParam);
}


#else


void bus_info(void)
{
  int i, count = 0;
  for(i = 0; i < max_slot; i++)
    {
      if(valid_socket(slot[i].fd))
      {     if (slot[i].name !=NULL)
      printf("%d: %s\n",i,slot[i].name); 
    else
      printf("%d: Unnamed\n",i); 
        count++;
      }
    }
  printf("%d of %d Slots in use \n", count, SLOTS);
  printf("Read-Ops: %d  Write-Ops: %d \n", read_ops, write_ops);
}

void  slot_info(int i)
{
  if (i >= 0 && i < SLOTS && valid_socket(slot[i].fd))
  { char hex[17]={0};
    if (slot[i].name !=NULL)
      printf("%s\n",slot[i].name); 
    else
      printf("Unnamed\n"); 
    chartohex(slot[i].from_addr,hex,8);
    printf("Slot %d Starting Address: #%s\n", i, hex);
    chartohex(slot[i].to_addr,hex,8);
    printf("Slot %d End Address     : #%s\n", i, hex);
    printf("Slot %d High Interrupt Mask: %X\n",i, (unsigned)slot[i].hi_mask);
    printf("Slot %d Low Interrupt Mask : %X\n",i, (unsigned)slot[i].low_mask);
  }
  else
    errormsg("Invalid slot number");
}

void process_stdin()
{
  char buffer[300] = {0};
  read(0, buffer, 300);
  if(strncmp(buffer, "help",4) == 0)
    debugs("%s",howto);
  else if(strncmp(buffer, "debug",5) == 0)
    debugflag = 1;
  else if(strncmp(buffer, "nodebug",7) == 0)
    debugflag = 0;
  else if(strncmp(buffer, "quit",4) == 0)
    exitflag = 1;
  else if(strncmp(buffer, "on",2) == 0 && !powerflag)
    power_all(1); 

  else if(strncmp(buffer, "off",3) == 0 && powerflag)
    power_all(0); 
  else if(strncmp(buffer, "reset",5) == 0)
    reset_all(); 
  else if(strncmp(buffer, "info",4) == 0)
    bus_info();
  else if(strncmp(buffer, "close",5) == 0)
    close_slot(atoi(&buffer[5]));
  else if(strncmp(buffer, "slot",4) == 0)
    slot_info(atoi(&buffer[4]));
  else if(strncmp(buffer, "signal",6) == 0)
    interrupt_all(atoi(&buffer[6]));
}

int main(int argc, char *argv[])
{
  printf("Type 'quit' to shutdown the motherboard\n");
  param_init(argc, argv);
  initialize_slots();
  create_server();
  do_commands();
  while(!exitflag)
    {
      build_read_fdset();
      select(max_fd+1, &read_fdset, &write_fdset, NULL, NULL);
      if(FD_ISSET(0, &read_fdset))
        process_stdin();
      if(FD_ISSET(mother_fd, &read_fdset))
        connect_new_device();
      process_read_fdset();
    }
  shutdown_server();
  fprintf(stdout,"Read-Ops: %d  Write-Ops: %d \n", read_ops, write_ops);
  return 0;
}
#endif
