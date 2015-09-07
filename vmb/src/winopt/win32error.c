#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "winopt.h"

#pragma warning(disable : 4996)

void win32_message(char *msg)
{
	MessageBox(hMainWnd,msg,"Message",MB_OK);
}

void win32_error(int line, char *message)
{ static char tmp[1000];
  sprintf(tmp,"ERROR (%s, %d): %s\r\n",program_name,line,message);
  win32_message(tmp);
}


void win32_error2(int line, char *message, char *info)
{ static char tmp[1000];
  sprintf(tmp,"ERROR (%s, %d): %s\r\n%s\r\n",program_name,line,message,info);
  win32_message(tmp);
}


void win32_fatal_error(int line, char *message)
{   win32_error(line, message);
	SendMessage(hMainWnd,WM_COMMAND,0,ID_FILE_EXIT);
	exit(1);
}

