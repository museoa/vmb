#include <stdio.h>

int main(void)
{  char name[100]; 
   register int i;
   printf("Whats your name: ");
   gets(name);
   for (i=0; i<20; i++)
     printf("Hello %s!\n",name);
   return 0;
}
