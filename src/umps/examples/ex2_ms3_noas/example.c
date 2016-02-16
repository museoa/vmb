extern void HALT();

void tprint(char p)
{
   int tmp;
   char* foo = (char*)0x1000025C;
   tmp = *(foo);
   *foo = p;
}

void mprint(char* chr)
{
    int i = 0;
    for(i=0;chr[i] != 0;i++)
	tprint(chr[i]);
}

char tget()
{
    long* keyfaultaddr = (long*)0x10000008;
    long* keydataaddr  = (long*)0x1000000C;
    long data = 0;
    char chr;
    while(data == 0)
    {
	data = *(keyfaultaddr);
    }
    if((data & 0xFF00) == 0x8000)
	return -1;
    data = *keydataaddr;
    chr = data & 0x00FF;
    return chr;
}

int main(void)
{
  char c;
  int isQuit = 0;
  while(1 == 1)
  {
     c = tget();
     if(c == -1)
     {
	mprint("\nerror!\n");
     }
     else if(c == '\r')
     {
     	mprint("\r\n");
     }
     else if(c == 'q')
     {
     	isQuit = 1;
	tprint('q');
	continue;
     }
     else if(c == '!')
     {
	tprint('!');
     	if(isQuit == 1)
	{
	   mprint("\nQuitting application!\n\n");
	   break;
	}
     }
     else
     {
	tprint(c);
     }
     isQuit = 0;
  }
  HALT();
  return(0);
}

