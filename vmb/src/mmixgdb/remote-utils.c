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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
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
#include "bus-util.h"

#ifdef WIN32	
/* needed for nonblocking events */
HWND hMainWnd = 0;

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

static int remote_debug = 0;

int remote_fd=-1;
int server_fd=-1;
int gdb_connected = 0;
static struct sockaddr_in sockaddr;

/* Open a connection to a remote debugger.
   NAME is the filename used for communication.  */

int remote_server (int port)
{
 
  int tmp;
#ifdef WIN32
  wsa_init();
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
  fprintf(stderr,"Connecting to gdb ...\n");
  if (bind (server_fd, (struct sockaddr *) &sockaddr, sizeof (sockaddr))
	|| listen (server_fd, 1))
  { perror ("Can't bind address");
    server_fd=-1;
    return 0;
  }
  return 1;
}



int dual_wait(int s1, int s2)
     /* return 0 if s1 is ready, 1 if s2 is ready */
{   fd_set readfs;    /* file descriptor set for read */
    int max_fd;
    struct timeval t;
    do {
      t.tv_usec=t.tv_sec=0;
      FD_ZERO(&readfs);
      if (s1>=0) FD_SET(s1, &readfs); 
      if (s1>=0) FD_SET(s2, &readfs); 
      if (s1>s2) max_fd = s1; else max_fd=s2;
      if (s1<0 || s2<0)
        select(max_fd+1, &readfs, NULL, NULL, &t);
      else
        select(max_fd+1, &readfs, NULL, NULL, NULL);
      if (FD_ISSET(s1, &readfs)) return 0;
      else if (FD_ISSET(s2, &readfs)) return 1;
      else if (s1<0) return 0;
      else if (s2<0) return 1;
    } while (1);
}


void single_wait(int s)
     /* return  if s is ready */
{   fd_set readfs;    /* file descriptor set for read */
    if (s<0) return;
    FD_ZERO(&readfs);
    FD_SET(s, &readfs); 
    do 
      select(s+1, &readfs, NULL, NULL,NULL);
    while (!FD_ISSET(s, &readfs));
}





int
remote_open (void)
{
  int tmp;
  tmp = sizeof (sockaddr);
  remote_fd = accept (server_fd, (struct sockaddr *) &sockaddr, &tmp);
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
	server_fd=-1;
	
    signal (SIGPIPE, SIG_IGN);	/* If we don't do this, then gdbserver simply
					   exits when the remote side dies.  */

#endif  

    /* Tell TCP not to delay small packets.  This greatly speeds up
       interactive response. */
    tmp = 1;
    setsockopt (remote_fd, IPPROTO_TCP, TCP_NODELAY,
	       (char *) &tmp, sizeof (tmp));

        /* Convert IP address to string.  */
    fprintf (stderr, "Remote debugging from host %s\n", 
    inet_ntoa (sockaddr.sin_addr));
    fprintf(stderr,"Connected\n");
    gdb_connected = 1;
  }
  return 1;
}

void
remote_close (void)
{
#ifdef WIN32
    closesocket(remote_fd);
	remote_fd = INVALID_SOCKET;
#else
    close(remote_fd);
	remote_fd=-1;
#endif 
	gdb_connected = 0;

}

/* flush output to gdb */
static char writebuf[BUFSIZ];
static int writebufcnt = 0;

static int flush(void)
{ int i=0;
 int error;
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
  int i;
  unsigned char csum = 0;
  char plus;
  int cc;

#if 1
  fprintf(stderr, "sending packet: $%s#\n",buf);
#endif

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

  cc = recv (remote_fd,&plus, 1,0);
  if (cc <= 0)
	{
	  if (cc == 0)
	  {  fprintf (stderr, "putpkt: Got EOF\n");
	     remote_close();
      }
	  else
	    perror ("putpkt");

	  return -1;
	}
  return (plus == '+');
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

  read_bufcnt = recv (remote_fd, read_buf, sizeof (read_buf),0);

  if (read_bufcnt <= 0)
    {
      if (read_bufcnt == 0)
	  {  fprintf (stderr, "readchar: Got EOF\n");
         remote_close();
          }
      else
	perror ("readchar");

      return -1;
    }

  read_bufp = read_buf;
  read_bufcnt--;
  return *read_bufp++;
}

int remote_interrupt(int s)
     /* check the remote side (non blocking) for an interrupt */
{
  return 0;
}
/* Read a packet from the remote machine, with error checking,
   and store it in BUF.  Returns length of packet, or negative if error. */

int
getpkt (char *buf)
{
  char *bp;
  unsigned char csum, c1, c2;
  int c;

#ifdef DEBUG
  fprintf(stderr, "receiving packet: ");
#endif


  while (1)
    {
      csum = 0;

      while (1)
	{
	  c = readchar ();
	  if (c == '$')
	    break;
	  if (remote_debug)
	    {
	      fprintf (stderr, "[getpkt: discarding char '%c']\n", c);
	      fflush (stderr);
	    }

	  if (c < 0)
	    return -1;
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
      send (remote_fd, "-", 1,0);
    }

  if (remote_debug)
    {
      fprintf (stderr, "getpkt (\"%s\");  [sending ack] \n", buf);
      fflush (stderr);
    }

#ifdef DEBUG
  fprintf(stderr, "$%s#\n",buf);
#endif


  send (remote_fd, "+", 1,0);
  if (remote_debug)

    {
      fprintf (stderr, "[sent ack]\n");
      fflush (stderr);
    }

  return bp - buf;
}


