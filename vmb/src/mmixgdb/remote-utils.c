/* Remote utility routines for the remote server for GDB.
   Copyright 1986, 1989, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
   2002
   Free Software Foundation, Inc.

   This file is part of GDB.

   adapded to the MMIX motherboard project 
   2005, by Martin Ruckert, ruckert@acm.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#define DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif
#include <signal.h>
#include <ctype.h>
#include "bus-arith.h" 
#include "error.h"
#include "buffers.h"
#include "gdb.h"

static int connect_to_gdb (int port);

/* we need only two buffers because we deal only with two threads
   and one thread will need at most one buffer.
*/

static queue free_buffers;
static queue cmd_buffers;

char *get_free_buffer(void)
{ return (char *)dequeue(&free_buffers);}
void put_free_buffer(char *buffer)
{ enqueue(&free_buffers,buffer);}
char *get_gdb_command(void)
{ return dequeue(&cmd_buffers);}


#ifdef WIN32
static DWORD WINAPI gdb_read_loop(LPVOID dummy)
#else
static void *gdb_read_loop(void *_dummy)
#endif
{ char *read_buffer;
  read_buffer = (char *)dequeue(&free_buffers); 
  while (1)
  { if (read_buffer==NULL)
    { perror("No free buffer available for gdb command");
      remote_close();
      return 0;
    }
    if (getpkt(read_buffer)<=0)
    { remote_close();
      return 0;
    }
    if (!async_gdb_command(read_buffer))
    { 
#ifdef DEBUG
  fprintf(stderr, "New command: >%s<\n",read_buffer);
#endif
      enqueue(&cmd_buffers,read_buffer);
      read_buffer = (char *)dequeue(&free_buffers); 
    }
  }
  remote_close();
  return 0;
}

#ifdef WIN32
static DWORD dwReadThreadId;
static HANDLE hReadThread;
#else
static int read_tid;
static pthread_t read_thr;
#endif


static char bufferA[PBUFSIZ];
static char bufferB[PBUFSIZ];

int gdb_init(int port)
     /* initialize the connection to gdb,
        return 1 on success, 0 on failure
     */
{ if (!connect_to_gdb(port)) return 0;
  /* start the read loop */
 init_queue(&free_buffers);
 init_queue(&cmd_buffers);
 enqueue(&free_buffers,bufferA);
 enqueue(&free_buffers,bufferB);
#ifdef WIN32
  { DWORD dwReadThreadId;
     hReadThread = CreateThread( 
            NULL,              // default security attributes
            0,                 // use default stack size  
            gdb_read_loop,        // thread function 
            NULL,             // argument to thread function 
            0,                 // use default creation flags 
            &dwReadThreadId);   // returns the thread identifier 
        // Check the return value for success. 
    if (hReadThread == NULL) 
      vmb_fatal_error(__LINE__, "Creation of bus thread failed");
/* in the moment, I really dont use the handle */
    CloseHandle(hReadThread);
  }
#else
   read_tid = pthread_create(&read_thr,NULL,gdb_read_loop,NULL);
#endif
  return 1;
}




#ifdef WIN32	
static void wsa_close(void)
{ WSACleanup();
}

void wsa_init(void)
{
  static int wsa_ready=0;
  WSADATA wsadata;
  if (wsa_ready) return;
  if(WSAStartup(MAKEWORD(1,1), &wsadata) != 0)
    perror("Unable to initialize Winsock dll");
  wsa_ready = 1;
  atexit(wsa_close);
}
#endif

static int remote_debug = 1;

#if !defined(INVALID_SOCKET)
#define INVALID_SOCKET  (~0)
#endif

#define valid_socket(socket)  ((socket) != INVALID_SOCKET)

SOCKET remote_fd=INVALID_SOCKET;
SOCKET server_fd=INVALID_SOCKET;
int gdb_connected = 0;
static struct sockaddr_in sockaddr;

/* Open a connection to a remote debugger.
   NAME is the filename used for communication.  */

int connect_to_gdb (int port)
{
 
#ifdef WIN32
  int tmp;
  wsa_init();
#else
  socklen_t tmp;
#endif

  server_fd = socket (PF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
  {  perror ("Can't open socket");
     return 0;
  }

      /* Allow rapid reuse of this port. */
  tmp = 1;
  setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp,
		  sizeof (tmp));

  sockaddr.sin_family = PF_INET;
  sockaddr.sin_port = htons ((unsigned short)port);
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  fprintf(stderr,"Connecting to gdb ...");
  if (bind (server_fd, (struct sockaddr *) &sockaddr, sizeof (sockaddr))
	|| listen (server_fd, 1))
  { perror ("Can't bind address\n");
    server_fd=INVALID_SOCKET;
    return 0;
  }
  tmp = sizeof (sockaddr);
  remote_fd = (int)accept (server_fd, (struct sockaddr *) &sockaddr, &tmp);
  if (remote_fd < 0 )
  {  perror ("Accept failed");
     return 0;
  }
  else
  { 
#ifdef WIN32
    closesocket(server_fd);
    server_fd = INVALID_SOCKET;
#else
    close(server_fd);
    server_fd=INVALID_SOCKET;	
    signal (SIGPIPE, SIG_IGN);	/* If we don't do this, then gdbserver simply
					   exits when the remote side dies.  */
#endif
    /* Tell TCP not to delay small packets.  This greatly speeds up
       interactive response. */
    tmp = 1;
    setsockopt (remote_fd, IPPROTO_TCP, TCP_NODELAY,
	       (char *) &tmp, sizeof (tmp));

        /* Convert IP address to string. 
    fprintf (stderr, "Remote debugging from host %s\n", 
    inet_ntoa (sockaddr.sin_addr));
	*/
    fprintf(stderr,"Connected\n");
    gdb_connected = 1;
  }
  return 1;
}

void
remote_close (void)
{
  if (valid_socket(remote_fd))
#ifdef WIN32
    closesocket(remote_fd);
#else
    close(remote_fd);
#endif 
    remote_fd = INVALID_SOCKET;
    gdb_connected = 0;
}

/* flush output to gdb */
static char writebuf[BUFSIZ];
static int writebufcnt = 0;

static int flush(void)
{ int i;
 int error=0;
#ifdef DEBUG
  fprintf(stderr, "To gdb:>");
  for (i=0;i<writebufcnt;i++)
    fprintf(stderr, "%c",writebuf[i]);
  fprintf(stderr, "<\n");
#endif
  i=0;
  while (i <writebufcnt)
  { error = send (remote_fd, writebuf+i, writebufcnt - i,0);
    if (error<0)
	{
	  perror ("flushing");
	  break;
	}
    else
      i = i+error;
  }
  writebufcnt=0;
  return error;
}

/* Write a char to remote GDB.  -1 if error.  */

static int
writechar (char c)
{
  writebuf[writebufcnt++]=c;
  if (writebufcnt>=BUFSIZ)
    return flush();
  else  
    return 1;
}


/* Send a packet to the remote machine, with error checking.
   The data of the packet is in BUF.  Returns >= 0 on success, -1 otherwise. */

int
putpkt (char *buf)
{
  int i, ok;
  unsigned char csum = 0;
  char *plus;

  writechar('$');

  for (i = 0; buf[i]!=0 ; i++)
    {
      csum += buf[i];
      writechar(buf[i]);
    }
  writechar('#');
  writechar(tohex ((csum >> 4) & 0xf));
  writechar(tohex (csum & 0xf));
  flush();

  /* Send it over and get a positive ack.  */
  plus = dequeue(&cmd_buffers);
  if (plus == NULL)
  {
     remote_close();
     return 0;
  }
  else if (*plus != '+')
  {  fprintf (stderr, "putpkt: Got %c (%2x)\n",*plus,*plus);
  }
  ok = (*plus== '+');
  put_free_buffer(plus);
  return ok;
}



/* Returns next char from remote GDB.  -1 if error.  */
  static unsigned char read_buf[BUFSIZ];
  static int read_bufcnt = 0;
  static unsigned char *read_bufp;

static int
readchar (void)
{

  if (read_bufcnt-- > 0)
    return *read_bufp++;

  read_bufcnt = recv (remote_fd, read_buf, BUFSIZ,0);

  if (read_bufcnt <= 0)
    {
      remote_close();
#ifdef DEBUG
      perror ("readchar");
#endif
      return -1;
    }
#ifdef DEBUG
  else
  { int i;
    fprintf(stderr, "From gdb:>");
    for (i=0;i<read_bufcnt;i++)
		if (read_buf[i]<=0x20 || read_buf[i]>=0x7F)
		   fprintf(stderr, "%c(%2X)",read_buf[i],read_buf[i]);
		else
		   fprintf(stderr, "%c",read_buf[i]);
    fprintf(stderr, "<\n");
  }
#endif

  read_bufp = read_buf;
  read_bufcnt--;
  return *read_bufp++;
}

/* Read a packet from the remote machine, with error checking,
   and store it in BUF.  Returns length of packet, or negative if error. */

int
getpkt (char *buf)
{
  char *bp;
  unsigned char csum, c1, c2;
  int c;

  while (1)
    {
      csum = 0;

      while (1)
	{
	  c = readchar ();
	  if (c == '$')
	    break;
          else if (c < 0)
	    return -1;
          else
	    { buf[0] = c; 
              buf[1]=0; 
              return 1;
	    }
	}

      bp = buf;
      while (1)
	{
	  c = readchar ();
	  if (c < 0)
	    return -1;
	  if (c == '#')
	    break;
	  *bp++ = c;
	  csum += c;
	}
      *bp = 0;

      c1 = fromhex ((char)readchar ());
      c2 = fromhex ((char)readchar ());

      if (csum == (c1 << 4) + c2)
	break;

      fprintf (stderr, "Bad checksum, sentsum=0x%x, csum=0x%x, buf=%s\n",
	       (c1 << 4) + c2, csum, buf);
      writechar('-');
      flush();
    }

  writechar('+');
  flush();
  return (int)(bp - buf);
}


