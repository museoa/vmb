#include <stdio.h>

int main(void)
{  char str[100]; 
   FILE *f;
   register int i;
   printf("filename: ");
   gets(str);
   f = fopen(str,"r");
   if (f==NULL)
     printf("Unable to open file %s\n",str);
   else
   {  while (fgets(str,99,f)!=NULL)
        printf("%s",str);
     fclose(f);
   }
   return 0;
}
