extern void HALT();

void tprint(char p)
{
   int tmp;
   char* foo = 0x1000025C;
   tmp = *(foo-4);
   *foo = p + tmp;
}


int main(void)
{
  int i;
  char message[] = "Hello world \n    how are you today \n  printing lots of text\n";
  for(i=0;message[i] != 0;i++)
  {
  	tprint(message[i]);
  }
  HALT();
  return(0);
}

