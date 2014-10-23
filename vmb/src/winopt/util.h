#ifndef _UTIL_H_
#define _UTIL_H_

extern void uint64tohex(uint64_t u, char *c);
extern uint64_t strtouint64(char *arg);
extern void inttochar(int val, unsigned char buffer[4]);
extern int chartoint(const unsigned char buffer[4]);
#if 0
static int strtoint(char *arg)
{ int r = 0;
  while(isspace(*arg)) arg++;
  if (strncmp(arg,"0x",2)==0 || strncmp(arg,"0X",2)==0) /* hex */
  { arg = arg+2;
	while (isxdigit(*arg))
	{ unsigned int x;
	  if (isdigit(*arg)) x = *arg - '0'; 
	  else if (isupper(*arg)) x = *arg - 'A' +10;
	  else x = *arg -'a'+10;
	  r = (r<<4) + x;
	  arg++;
	}
  }
  else /* decimal */
      while (isdigit(*arg))
	  {unsigned int d;
       d = *arg -'0';
	   r = r*10+d;
	   arg++;
	  }
  return r;
}


static double strtodouble(char *arg)
{ double r = 0;
  while(isspace(*arg)) arg++;
  while (isdigit(*arg))
	  {unsigned int d;
       d = *arg -'0';
	   r = r*10+d;
	   arg++;
	  }
  if (*arg=='.')
  { double f=0.1;
    arg++;
    while (isdigit(*arg))
	{  unsigned int d;
       d = *arg -'0';
	   r = r+d*f;
	   f=f*0.1;
	   arg++;
	}
  }
  return r;
}

#endif
#endif