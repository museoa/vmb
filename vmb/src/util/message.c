/*
    Copyright 2005 Martin Ruckert
    
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

#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include "win32main.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include "error.h"
#include "message.h"
#include "bus-arith.h"

/* functions to send and receive a message */
static int write_socket(int socket, int blocking, unsigned char *msg, int size)

/* write buffer to the socket either completely or, if blocking is false, not at all */
{ int snd = 0;

  while(snd < size)
  { static int i;
    i = send(socket, &msg[snd], size-snd,0);
    if (i<0)
    {
#ifdef WIN32
	  i=WSAGetLastError ();
	  if (i == WSAESHUTDOWN || i == WSAECONNABORTED)
      { bus_unregister(socket);
	    bus_disconnect(socket);
		return -1;
	  }
      else if (i == WSAEWOULDBLOCK )
#else
      if (errno == EAGAIN)
#endif	
      { if (!blocking && snd==0) return 0;
        else 
        { fd_set writefs;
          FD_ZERO(&writefs);
	  FD_SET((unsigned)socket, &writefs); 
	  select(socket+1, NULL,&writefs, NULL, NULL);
        }
      }
      else
	  { bus_disconnect(socket);
        return -1;
	  }
    } 
    else
      snd += i;
  }
  return 1;
}

int send_msg(int socket, int blocking,
         unsigned char type,
         unsigned char size,
         unsigned char slot,
         unsigned char id,
         unsigned int  time,
         unsigned char address[8],
         unsigned char *payload)
{
	unsigned char buf[MAXMESSAGE] = {0}, *msg=buf;
        int msg_size;
        
        if (socket<0)
          return -1;

	*msg++ = type;
	*msg++ = size;
	*msg++ = slot;
	*msg++ = id;
	msg_size = message_size(buf);

	if(type & TYPE_TIME)
	{
		inttochar(time, msg);
		msg += 4;
	}

	if(type & TYPE_ADDRESS)
	{
		memmove(msg, address, 8);
		msg += 8;
	}

	if(type & TYPE_PAYLOAD)
	{
		memmove(msg, payload, (size+1)*8);
	}

	return write_socket(socket,blocking,buf,msg_size);
}

static int read_socket(int socket, int blocking, unsigned char *msg, int size)

/* read a complete message from the socket */
{ int rcv = 0;

  while(rcv < size)
  { int i;
    i = recv(socket, &msg[rcv], size-rcv,0);
    if (i<0)
    {
#ifdef WIN32
	  i=WSAGetLastError ();
	  if (i == WSAESHUTDOWN || i == WSAECONNABORTED)
      { bus_unregister(socket);
	    bus_disconnect(socket);
		return -1;
	  }
      else if (i == WSAEWOULDBLOCK)
#else
      if (errno == EAGAIN)
#endif
      { if (!blocking && rcv==0) return 0;
      }
      else
      { bus_disconnect(socket);
        return -1;
	  }
    }
    else if (i==0) 
    { bus_unregister(socket);
#ifdef WIN32
#else
      bus_disconnect(socket);
#endif
 	  return -1;
    } 
    else
      rcv += i;
    if (rcv<size) 
    { fd_set readfs;
      FD_ZERO(&readfs);
      FD_SET((unsigned)socket, &readfs);
      select(socket+1, &readfs, NULL, NULL, NULL);
    }
  }
  return rcv;
}



int receive_msg(int socket, int blocking,
         unsigned char *type,
         unsigned char *size,
         unsigned char *slot,
         unsigned char *id,
         unsigned int  *time,
         unsigned char address[8],
         unsigned char *payload)
{
	//recieve header and compute messgage size
	unsigned char buf[MAXMESSAGE] = {0}, *msg=buf;
        int msg_size;
        int rcv;

        if (socket<0)
          return -1;
        rcv = read_socket(socket,blocking,msg,4);
        if (rcv <= 0)
	  return rcv;
	msg_size = message_size(msg);

	//recieve rest of message
        if (msg_size > 4)
        { rcv = read_socket(socket,1,msg+4,msg_size-4);
          if (rcv <0 )
	    return rcv;
        }
	//transfer data to pointers
	*type = *msg++;
	*size = *msg++;
	*slot = *msg++;
	*id   = *msg++;

	if(*type & TYPE_TIME)
	  { *time = chartoint(msg); msg=msg+4; }

	if(*type & TYPE_ADDRESS)
	  { memmove(address, msg, 8); msg=msg+8;}

	if(*type & TYPE_PAYLOAD)
	  memmove(payload,msg, (*size+1) * 8);

	return 1;
}

int message_size(unsigned char msg[4])
{
	int size = 4;
	if(msg[0] & TYPE_TIME)
		size += 4;

	if(msg[0] & TYPE_ADDRESS)
		size += 8;

	if(msg[0] & TYPE_PAYLOAD)
		size += (msg[1]+1)*8;

	return size;
}




/* functions to connect, register, unregister, and disconnect */

int bus_connected=0;

int bus_connect(char *hostname,int port)
{ int fd = 0;
  struct sockaddr_in host_addr;
  static char localhost[] = "localhost";

  if (hostname == NULL)
    hostname = localhost;
  else if (hostname[0]==0)
    hostname = localhost;

  fd = socket( PF_INET, SOCK_STREAM, 0);

  if (!valid_socket(fd))
    return fd;



  /* make the socket non blocking */

#ifdef WIN32
  if (hMainWnd)
  WSAAsyncSelect(fd, hMainWnd, WM_SOCKET, FD_READ | FD_CONNECT |FD_ACCEPT | FD_CLOSE);
  else
  {	 WSAEVENT  e;
	 e = WSACreateEvent();
	 WSAEventSelect (fd, e,FD_READ | FD_CONNECT | FD_ACCEPT | FD_CLOSE);
  }
#else
  { int flags;
    flags = fcntl(fd,  F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL,flags);
  }
#endif

{ 
  unsigned long server_ip;

  server_ip = inet_addr(hostname);
  if (server_ip == INADDR_NONE)
  { struct hostent *hp;
    hp = gethostbyname(hostname);
    if (hp==NULL)
    { server_ip = 0;
      return INVALID_SOCKET;
    }
    memcpy(&server_ip,hp->h_addr,sizeof(server_ip));
  }
  host_addr.sin_addr.s_addr = server_ip;
}

  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons((unsigned short)port);
{ int i;
  i = connect(fd,(struct sockaddr *)&host_addr,sizeof(host_addr));
  if (i < 0 )
    { 
#ifdef WIN32
      if (WSAGetLastError()!=WSAEWOULDBLOCK)
#else
      if (errno!=EINPROGRESS)
#endif
	  		  return INVALID_SOCKET;
    }

  /* wait until it is writable, then the connection has succeeded */
  { 
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET((unsigned)fd, &write_set);
    if (select (fd+1, NULL, &write_set, NULL, NULL)<0)
         return  INVALID_SOCKET;
  }
}
  {
    /* Tell TCP not to delay small packets.  This greatly speeds up
       interactive response. */
    int tmp = 1;
    setsockopt (fd, IPPROTO_TCP, TCP_NODELAY,
		(char *) &tmp, sizeof (tmp));

  }
  bus_connected=1;
  return fd; 
}

int bus_register(int socket,
                unsigned char address[8],
                unsigned char limit[8],
                unsigned int hi_mask, unsigned int low_mask,

				char *name)
{ unsigned char size;
  unsigned char msg[MAXMESSAGE] = {0};
  if (socket<0)
    return -1;
  if (address!=NULL && limit!=NULL)
  { memmove(msg, address, 8);
    memmove(msg+8, limit, 8);
  }
  else
    memset(msg,0,16);
  inttochar(hi_mask, &msg[16]);
  inttochar(low_mask, &msg[20]);

  size = 2;

  if (name!=NULL)

  { int n;

    n = strlen(name);

	if ((n/8+1)> 255-2) n = 255-2;

	strncpy(msg+24,name,n*8);

    size += n;

  }
  /* send bus register message */
  return send_msg(socket, 1, TYPE_BUS|TYPE_PAYLOAD, size, 0, ID_REGISTER, 0, 0, msg);
}

int bus_unregister(int socket)
{ if (socket<0)
    return -1;
  /* TODO per Message abmelden */
#ifdef WIN32
  return shutdown(socket,SD_BOTH);
#else
  return shutdown(socket,SHUT_RDWR);
#endif
}

int bus_disconnect(int socket)
{  bus_connected = 0;

if (valid_socket(socket))
#ifdef WIN32
  return closesocket(socket);
#else
  return close(socket);
#endif
  return -1;
}

