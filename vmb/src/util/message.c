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
#pragma warning(disable : 4996)
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
static int write_socket(int socket, unsigned char *msg, int size)
/* write buffer to the socket either completely or, if blocking is false, not at all */
{ int snd = 0;
  while(snd < size)
  { int i;
    i = send(socket, &msg[snd], size-snd,0);

    if (i<=0)
    { bus_disconnect(socket);
      return -1;
    }
    else
      snd += i;
  }
  return 1;
}

int send_msg(int socket,
         unsigned char type,
         unsigned char size,
         unsigned char slot,
         unsigned char id,
         unsigned int  time,
         unsigned char address[8],
         unsigned char *payload)
{ unsigned char buf[MAXMESSAGE] = {0}, *msg=buf;
  int msg_size;
        
  if (!valid_socket(socket))
    return -1;

  if (address == NULL)
	type = type & ~TYPE_ADDRESS;
  if (payload == NULL)
	type = type & ~TYPE_PAYLOAD;

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
  return write_socket(socket,buf,msg_size);
}
#ifdef WIN32
HANDLE  recv_event;
#endif

static int read_socket(int socket, unsigned char *msg, int size)
/* read a complete message from the socket */
{ int rcv = 0;
  while(rcv < size)
  { int i;
    i = recv(socket, &msg[rcv], size-rcv,0);
    if (i<0) /* error */
    { 
#ifdef WIN32 
		i = WSAGetLastError();
		if (i == WSAEWOULDBLOCK)
		{  /* if the socket was used in nonblocking mode (motherboadr) 
		      it might return 0 */
			return 0;
		}
#endif		
		bus_disconnect(socket);
	  return -1;
	}
    else if (i==0) /* connection has closed */
    { 
/* there was a #ifdef WIN32 #else here to skip the disconnect with WIN32 */
      bus_disconnect(socket); 
 	  return -1;
    } 
    else
      rcv += i;
  }
  return rcv;
}



int receive_msg(int socket,
         unsigned char *type,
         unsigned char *size,
         unsigned char *slot,
         unsigned char *id,
         unsigned int  *time,
         unsigned char address[8],
         unsigned char *payload)
{ /* recieve header and compute messgage size */
  unsigned char buf[MAXMESSAGE] = {0}, *msg=buf;
  int msg_size;
  int rcv;

  if (!valid_socket(socket))
    return -1;
  rcv = read_socket(socket,msg,4);
  if (rcv <= 0)
    return rcv;
  msg_size = message_size(msg);

  /*recieve rest of message */
  if (msg_size > 4)
  { rcv = read_socket(socket,msg+4,msg_size-4);
    if (rcv <0 )
      return rcv;
  }
  /* transfer data to pointers */
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
{ int size = 4;
  if(msg[0] & TYPE_TIME)
	size += 4;
  if(msg[0] & TYPE_ADDRESS)
	size += 8;
  if(msg[0] & TYPE_PAYLOAD)
	size += (msg[1]+1)*8;
  return size;
}




/* functions to connect, register, unregister, and disconnect */

static int bus_connected=0;



int bus_connect(char *hostname,int port)
{ int fd = 0;
  struct sockaddr_in host_addr;
  static char localhost[] = "localhost";
  if (bus_connected) return INVALID_SOCKET;

  if (hostname == NULL)
    hostname = localhost;
  else if (hostname[0]==0)
    hostname = localhost;

#ifdef WIN32
  {	WSADATA wsadata;
	if(WSAStartup(MAKEWORD(1,1), &wsadata) != 0)
    {  vmb_error(__LINE__,"Unable to initialize Winsock dll");
	   return INVALID_SOCKET;
	}
  }
#endif
  fd = (int)socket( PF_INET, SOCK_STREAM, 0);

  if (!valid_socket(fd))
  {
#ifdef WIN32
	WSACleanup();
#endif
    return INVALID_SOCKET;
  }

#if 0
   /* create an event corresponding to the socket */
	 recv_event = CreateEvent(NULL,False,False,NULL);
	 if (recv_event == NULL)
       return INVALID_SOCKET;
	 WSAEventSelect (fd, e,FD_READ );
#endif

{ 
  unsigned long server_ip;

  server_ip = inet_addr(hostname);
  if (server_ip == INADDR_NONE)
  { struct hostent *hp;
    hp = gethostbyname(hostname);
    if (hp==NULL)
    { server_ip = 0;
#ifdef WIN32
      WSACleanup();
#endif
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
	WSACleanup();
#endif
    return INVALID_SOCKET;
  }
    /* wait until it is writable, then the connection has succeeded */
  { 
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET((unsigned)fd, &write_set);
    if (select (fd+1, NULL, &write_set, NULL, NULL)<0)
    {
#ifdef WIN32
	  WSACleanup();
#endif
      return INVALID_SOCKET;
    }
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

    n = (int)strlen(name);
    if ((n/8+1)> 255-3)
       n = (255-4)*8;

    strncpy((char *)(msg+24),name,n*8);
    size += n/8+1;
  }
  /* send bus register message */
  return send_msg(socket, TYPE_BUS|TYPE_PAYLOAD, size, 0, ID_REGISTER, 0, 0, msg);
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
{ 
  if (bus_connected)
  { bus_connected = 0;

#ifdef WIN32
    if (valid_socket(socket))
       closesocket(socket);
	WSACleanup();
#else
    if (valid_socket(socket))
      close(socket);
#endif
  }
  return 0;
}

