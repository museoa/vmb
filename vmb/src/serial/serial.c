/*
    Copyright 2011  Martin Ruckert
    
    ruckertm@acm.org

    This file is part of the Virtual Motherboard project

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
char version[]="$Revision: 1.2 $ $Date: 2011-04-26 19:55:40 $";

char howto[] =
"The program will contact the motherboard at [host:]port\r\n"
"and register itself with the given address and interrupt.\r\n"
"Then, the program will create a pseudo tty and be ready to\r\n"
"send and receive bytes from the virtual board sending\r\n"
"connecting the send and receive channel to the pseudo tty.\r\n"
"\r\n"
"Reading from the tty:\r\n"
"At offset 0, it will provide a read/write octabyte\r\n"
"in the following format:\r\n"
"\r\n"
"   XX00 00YY 0000 00ZZ\r\n"
"\r\n"
"The XX byte signals errors it will be 0x80 if an error occured\r\n"
"and 0x00 otherwise.\r\n"
"\r\n"
"The YY byte contains the number of characters read from \r\n"
"the tty since the last read operation.  This should be 0\r\n"
"if no new character was received and 1 if one charcter was\r\n"
"received. Any other value will indicate that character were\r\n"
"lost since the last read operation.\r\n"
"\r\n"
"The ZZ byte contains the last character received. It is valid only\r\n"
"if YY is not zero.\r\n"
"\r\n"
"The complete ocatbyte will be reset to zero after a read operation to ZZ.\r\n"
"\r\n"
"Reading a YY byte equal to zero means there is no ZZ byte available\r\n"
"Reading a YY value that is zero, will generate an interrupt\r\n"
"as soon as YY is becomming non zero, if interrupts are enabled.\r\n"
"If the application does not read YY, there will be no interrupts.\r\n"
"\r\n"
"Writing to the tty:\r\n"
"At offset 8, it will provide a read/write octabyte\r\n"
"in the following format:\r\n"
"\r\n"
"   XX00 00YY 0000 00ZZ\r\n"
"\r\n"
"The XX byte signals errors it will be 0x80 if an error occured\r\n"
"and 0x00 otherwise.\r\n"
"\r\n"
"The YY byte contains the number of characters that were written\r\n"
"to this address since the last output. This should be 0\r\n"
"if the address is ready to receive a character or 1 if the hardware\r\n"
"is bussy with writing a byte. Any other value will indicate that characters were\r\n"
"lost since the last write operation.\r\n"
"\r\n"
"The ZZ byte contains the last character received. It is valid only\r\n"
"if YY is not zero.\r\n"
"\r\n"
"The complete ocatbyte will be reset to zero after a byte is output.\r\n"
"\r\n"
 "Reading a YY byte equal to zero means you can write one byte to ZZ to\r\n"
"produce output. Reading a YY value that is not zero, will generate an\r\n"
"interrupt as soon as YY is returning to zero, if interrupts are enabled.\r\n"
"If the application does not read YY, there will be no interrupts.\r\n"
;

#ifdef WIN32
#include <windows.h>
extern HWND hMainWnd;
#else
#define _XOPEN_SOURCE 600
#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#endif

#include "vmb.h"
#include "error.h"
#include "bus-arith.h"
#include "param.h"
#include "option.h"

/* options */
extern int rinterrupt, winterrupt, rdisable, wdisable;
static int wrequested=0, rrequested=0;

device_info vmb;


#define SERIAL_MEM	0x10
unsigned char smem[SERIAL_MEM];
#define RXX (smem[0])
#define RYY (smem[3])
#define RZZ (smem[7])
#define WXX (smem[8])
#define WYY (smem[11])
#define WZZ (smem[15])

/* official copies of RYY, RXX, RZZ */
static unsigned char rxx=0,ryy=0, rzz=0;

/* events/actions: 
    getpayload from the vmb thread
      if needed take data from input buffer
      return data from the memory
      
    putpayload from the vmb thread
      put data into memory
      if a character needs to be written
      place it in the output buffer
      
    input from the ptty
      place input in the input buffer
      signal avalable data if required
*/

/* two circular buffers for io */
#define BUF_MAX (1<<6)
#define BUF_MASK (BUF_MAX-1)
struct buf {
  unsigned char buf[BUF_MAX];
  int head, tail;
#ifdef WIN32
  HANDLE hbuf;
  CRITICAL_SECTION   buf_section;
#else
  pthread_mutex_t buf_mutex;
  pthread_cond_t buf_cond;
#endif
} inbuf, outbuf;

#ifdef WIN32
#define bufacquire(buf)  EnterCriticalSection (buf->buf_section)
#else
#define bufacquire(buf)  ((pthread_mutex_lock(&buf->buf_mutex))?	\
		       (vmb_error(__LINE__,"Locking buf mutex failed"), \
			pthread_exit(NULL)):0)
#endif 

#ifdef WIN32
#define bufrelease(buf)  LeaveCriticalSection (buf->buf_section);
#else
#define bufrelease(buf)  ((pthread_mutex_unlock(&buf->buf_mutex))? \
                       (vmb_error(__LINE__,"Unlocking buf mutex failed"), \
			pthread_exit(NULL)):0)
#endif

static void clear(struct buf *buf)
{ bufacquire(buf);
  buf->head=buf->tail=0;
  bufrelease(buf);
}

static int is_empty(struct buf *buf)
{ return buf->head==buf->tail;
}

static int is_full(struct buf *buf)
{ return ((buf->tail+1)&BUF_MASK)==buf->head;
}

static void clean_up_buffer_mutex(void *buf)
{ pthread_mutex_unlock(&(((struct buf *)buf)->buf_mutex)); /* needed if canceled waiting */
}
static void wait_not_empty(struct buf *buf)
{ 
 #ifndef WIN32
  { int rc;
    rc = pthread_mutex_lock(&buf->buf_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Locking buffer mutex failed");
      pthread_exit(NULL);
    }
    pthread_cleanup_push(clean_up_buffer_mutex,buf);
    /* in the meantime power might be on */
    while (is_empty(buf))
    rc = pthread_cond_wait(&buf->buf_cond,&buf->buf_mutex);
    pthread_cleanup_pop(1);
  }
#else
  /* in the meantime the buf might not be empty */
  while (is_empty(buf))
    WaitForSingleObject(buf->hbuf,INFINITE);
#endif
}

static int get(struct buf *buf)
/* return -1 on failure the character on success */
{ unsigned char c;
  bufacquire(buf);
  if (is_empty(buf))
    { bufrelease(buf);
      return -1;
    }
  c = *(buf->buf+buf->head);
  buf->head = (buf->head+1)&BUF_MASK;
  bufrelease(buf);
  return c;
}

static int getall(struct buf *buf, unsigned char b[], int size)
/* return -1 on failure the number if charactes written to b on success */
{ int i=0;
  bufacquire(buf);
  while (i<size && ! is_empty(buf))
    { b[i] = *(buf->buf+buf->head);
    buf->head = (buf->head+1)&BUF_MASK;
    i++;
  }
  bufrelease(buf);
  return i;
}

static int put(struct buf *buf, unsigned char c)
/* return -1 on failure, 1 on success, put c into buffer */
{ int next;
  bufacquire(buf);
  next = (buf->tail+1)&BUF_MASK;
  if (next==buf->head)
    { bufrelease(buf);
      return -1;
    }
  *(buf->buf+buf->tail)=c;
  buf->tail=next;

#ifdef WIN32
  bufrelease(buf);
  SetEvent (buf->hbuf);
#else
  pthread_cond_signal(&buf->buf_cond);
  bufrelease(buf);
#endif  
return 1;
}

static int ttyfd;



static void *write_thread(void *dummy)
/* wait for outbuf to be not empty and write (blocking)
  the content to ttyfd, if the thread blocks in the end the
  device blocks to the application, which is what we want.
*/
{ do 
    { int size, i=0;
    unsigned char buf[BUF_MAX];
    wait_not_empty(&outbuf);
    size=getall(&outbuf,buf,BUF_MAX);
    while (i<size)
    { int ret;
      ret = write(ttyfd,buf+i,size-i);
      if (ret<=0)
      { vmb_debug(VMB_DEBUG_ERROR,"Error Writing to TTY");
        return 0;
      }
      i=i+ret;
    }
  } while (1);
}

void set_read(unsigned char c)
{ rzz=c;
  if (ryy<0xFF) ryy++;
  if (ryy>1) rxx=0x80;
}

static void *read_thread(void *dummy)
/* wait (blocking) for a character to become available
   on the tty, if so put it in the inbuf, if the
   inbuf gets full, take out a character to make room
   and push it to rzz, incrementing ryy, and possibly
   setting rxx,  creating errors on the vmb side.
   then put the new character in the inbuf.
 */
{ do 
  { int size;
    unsigned char buf[BUF_MAX];
  start:
    size = read(ttyfd,buf,BUF_MAX);
    if (size==0) 
      { vmb_debug(VMB_DEBUG_PROGRESS,"End of Input Reading from TTY");
        return 0;
      }
    else if (size<0)
      {
#if 0
        if (errno==EIO)
	  { int i, flags;
            static int old_flags=-1;
            i=ioctl(ttyfd,TIOCMGET,&flags);
	    if (i!=0)
	    { vmb_debugi(VMB_DEBUG_ERROR,"Reading Modem Lines returned %d", i);
              vmb_debugi(VMB_DEBUG_ERROR,"Error %d Reading Modem Lines", errno);
	    }
            if (flags!=old_flags)
	      { vmb_debugi(VMB_DEBUG_ERROR,"Modem Lines from TTY %X", flags);
                old_flags=flags;
	      }
            usleep(10000);
            goto start;
	  }
	else
#endif
        vmb_debugi(VMB_DEBUG_ERROR,"Error %d Reading from TTY", errno);
        return 0;
      }
    else
      { int i;
        vmb_debugi(VMB_DEBUG_PROGRESS,"Reading %i byte from TTY", size);
        for (i=0; i<size;i++)
	  { if (is_full(&inbuf))
	      { set_read(get(&inbuf));
                vmb_debug(VMB_DEBUG_NOTIFY,"Input buffer full");
	      }
            if (rrequested&!rdisable&is_empty(&inbuf))
	      { rrequested=0;
                vmb_raise_interrupt(&vmb,rinterrupt);
                vmb_debug(VMB_DEBUG_PROGRESS,"Read Interrupt sent");
	      }
	    put(&inbuf,buf[i]);
            if (isprint(buf[i]))
              vmb_debugi(VMB_DEBUG_INFO,"Reading %c from TTY", buf[i]);
	    else
              vmb_debugi(VMB_DEBUG_INFO,"Reading %02X from TTY", buf[i]);
	  }
      }
  } while(1);
}


#ifdef WIN32
#else
static int read_tid;
static pthread_t read_thr;
static int write_tid;
static pthread_t write_thr;
#endif

static void init_threads(void)
{
#ifdef WIN32
  vmb_critical_error(__LINE__,"Threads not yet implemented");
#else
  pthread_mutex_init(&inbuf.buf_mutex,NULL);
  pthread_cond_init(&inbuf.buf_cond,NULL);
  pthread_mutex_init(&outbuf.buf_mutex,NULL);
  pthread_cond_init(&outbuf.buf_cond,NULL);

   read_tid  = pthread_create(&read_thr,NULL,read_thread,NULL);
   write_tid = pthread_create(&write_thr,NULL,write_thread,NULL);
#endif
}



unsigned char *serial_get_payload(unsigned int offset,int size)
{ vmb_debug(VMB_DEBUG_INFO,"Reading serial information");
  RXX=rxx,RYY=ryy,RZZ=rzz;
  if (offset<=7 && offset+size>7) /* reading input */
    { if (!is_empty(&inbuf))
	{ if (RYY<0xFF) RYY++;
          if (RYY>1) RZZ=0x80;
          RZZ=get(&inbuf); 
          vmb_debugi(VMB_DEBUG_PROGRESS,"reading charcter 0x%02X",RZZ);
	}
      else
         vmb_debug(VMB_DEBUG_NOTIFY,"no charcter available");
      rxx=ryy=rzz=0; /* reading clears the octabyte */
    }
  if (offset<=3 && offset+size>3 && RYY==0) rrequested=1;
  if (offset<=11 && offset+size>11 && WYY!=0) wrequested=1;
  return smem+offset;
}

void serial_put_payload(unsigned int offset,int size, unsigned char *payload)
{ vmb_debug(VMB_DEBUG_INFO,"Writing serial information");
  if (offset<=15 && offset+size>15) /* writing output */
    { WZZ= payload[15-offset]; /* the only writable byte the rest is ignored */
      if (is_full(&outbuf))
      {  WXX=0x80;
         if (WYY<0xFF) WYY++;
         vmb_debugi(VMB_DEBUG_NOTIFY,"unable to write charcter 0x%02X",WZZ);
      }
      else
	{ WXX=0;
	  put(&outbuf,WZZ);
          WYY=0;
          vmb_debugi(VMB_DEBUG_PROGRESS,"writing charcter 0x%02X",WZZ);
          WZZ=0;
	}
    }
}


void serial_null(void)
{ memset(smem,0,sizeof(smem));
  rxx=0,ryy=0, rzz=0;
  wrequested=0, rrequested=0;
  clear(&inbuf);
  clear(&outbuf);
}

void serial_reset(void)
{ vmb_debug(VMB_DEBUG_PROGRESS,"Serial reset");
  serial_null();
}
void serial_poweron(void)
/* this function is called when the virtual power is turned off */
{ vmb_debug(VMB_DEBUG_PROGRESS,"Serial power on");
  serial_null();
#ifdef WIN32
  PostMessage(hMainWnd,WM_VMB_ON,0,0);
#endif
}

void serial_poweroff(void)
/* this function is called when the virtual power is turned off */
{ vmb_debug(VMB_DEBUG_PROGRESS,"Serial power off");
  serial_null();
#ifdef WIN32
  PostMessage(hMainWnd,WM_VMB_OFF,0,0);
#endif
}


void serial_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ 
  vmb_debug(VMB_DEBUG_PROGRESS,"Serial disconnected");
  close(ttyfd);
#ifdef WIN32
  PostMessage(hMainWnd,WM_VMB_DISCONNECT,0,0);
#endif
}





static char *slave_tty_path;

int create_pseudo_tty(void)
/* returns fd of master device
   sets path name of slave tty in slave_tty_path
*/
{
  int fd;

#if 1
  fd = posix_openpt(O_RDWR /* | O_NOCTTY */ );
  if (fd == -1)
    vmb_debug(VMB_DEBUG_ERROR,"Error opening a pseudo terminal.\n");

  if (grantpt(fd)!=0)
    vmb_debug(VMB_DEBUG_ERROR,"Error opening slave pseudo terminal.\n");

  if (unlockpt(fd)!=0)
    vmb_debug(VMB_DEBUG_ERROR,"Error unlocking pseudo terminal.\n");


  slave_tty_path = ptsname(fd);
  if (slave_tty_path==NULL)
    vmb_debug(VMB_DEBUG_ERROR,"unable to get Name of slave tty device.\n");

 
#else
  fd = open("/dev/ptypf", O_RDWR);
  if (fd == -1)
    vmb_debug(VMB_DEBUG_ERROR,"Error opening a pseudo terminal /dev/ptypf \n");
  slave_tty_path =  "/dev/ttypf";
#endif

 /* to remove echos we do this 
     input with echo keeps the thing alive
     input with cat file > /dev/pts/4 produces an end of file
     seems ok !
  */
  { struct termios tios;
#if 0   
#define BAUDRATE B38400
 
        memset(&tios,0, sizeof(tios));
        tios.c_cflag = BAUDRATE | CRTSCTS | CS8 | CREAD;
        tios.c_iflag = IGNPAR;
        tios.c_oflag = 0;
        
        /* set input mode (non-canonical, no echo,...) */
        tios.c_lflag = 0;
         
        tios.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        tios.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */
        
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&tios);

        /* allow the process to receive SIGIO */
        fcntl(fd, F_SETOWN, getpid());

#else
    tcgetattr(fd,&tios);
    /* make it a raw tty (like cfmakeraw(&tios) ) */
    tios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                      | INLCR | IGNCR | ICRNL | IXON);
    tios.c_oflag &= ~OPOST;
    tios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tios.c_cflag &= ~(CSIZE | PARENB);
    tios.c_cflag |= CS8;
    tios.c_cc[VMIN]=1;
    tios.c_cc[VTIME]=0;
    tcsetattr(fd,TCSANOW,&tios);
#endif

  }
  return fd;
}


void serial_init(void)
{ ttyfd =  create_pseudo_tty();
  if (ttyfd<0) vmb_fatal_error(__LINE__,"Unable to open pseudo tty");
  fprintf(stderr,"Created device %s\n",slave_tty_path);
  serial_null();
}
   
void init_device(device_info *vmb)
{ vmb_debug(VMB_DEBUG_PROGRESS,"Serial initializing");
  serial_init();
  vmb_size = SERIAL_MEM;
  vmb->poweron=serial_poweron;
  vmb->poweroff=serial_poweroff;
  vmb->disconnected=serial_disconnected;
  vmb->reset=serial_reset;
  vmb->terminate=vmb_terminate;
  vmb->put_payload=serial_put_payload;
  vmb->get_payload=serial_get_payload;
}

static
void sighndl(int signo)
{
        /* signal(signo, sighndl); */
	fprintf(stderr, "Got signal %d\n", signo);
	fflush(stderr);
        exit(1);
}


int main(int argc, char *argv[])
{
  signal(SIGINT,sighndl);
  signal(SIGTERM,sighndl);
  /* signal(SIGKILL,sighndl); a bit dangerous */
  signal(SIGHUP, sighndl);
  signal(SIGIO, sighndl);

  param_init(argc, argv);
  if (vmb_verbose_flag) vmb_debug_mask=0;
  vmb_debugs(VMB_DEBUG_INFO, "%s ",vmb_program_name);
  vmb_debugs(VMB_DEBUG_INFO, "%s ", version);
  vmb_debugs(VMB_DEBUG_INFO, "host: %s ",host);
  vmb_debugi(VMB_DEBUG_INFO, "port: %d ",port);
  close(0);
  init_device(&vmb);
  vmb_debugi(VMB_DEBUG_INFO, "address hi: %x",HI32(vmb_address));
  vmb_debugi(VMB_DEBUG_INFO, "address lo: %x",LO32(vmb_address));
  vmb_debugi(VMB_DEBUG_INFO, "size: %x ",vmb_size);
  
  init_threads();

  vmb_connect(&vmb, host,port); 
  vmb_register(&vmb,HI32(vmb_address),LO32(vmb_address),vmb_size,
               0, 0, vmb_program_name);
#if 0
  while (vmb.connected)
  { fd_set except_set;
    int ready;
    FD_ZERO(&except_set);
    FD_SET((unsigned)ttyfd, &except_set);
    ready = select (ttyfd+1, NULL, NULL, &except_set, NULL);
    if (ready<0)
      perror("select()");
    else if (FD_ISSET(ttyfd,&except_set))
    { int arg;
      vmb_debug(VMB_DEBUG_PROGRESS,"Exception on TTY\n");
      if(ioctl(ttyfd, TIOCMGET, &arg) < 0)
            perror("TIOCMGET ioctl failed");
      else
      {  vmb_debugi(VMB_DEBUG_PROGRESS,"ioctl %08X\n",arg);
      }
    }
  }
#else
  vmb_wait_for_disconnect(&vmb);
#endif
  pthread_kill(read_thr,SIGTERM);
  pthread_kill(write_thr,SIGTERM);
  vmb_debug(VMB_DEBUG_PROGRESS, "Serial exit.");
  return 0;
}
