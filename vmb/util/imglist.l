
%{
#include <stdio.h>
char *prefix;
%}


%x IMG SRC

%%

\<[iI][mM][gG]       BEGIN IMG;
\n              ;
.               ;

<IMG>[sS][rR][cC][[:space:]]*=[[:space:]]*    BEGIN SRC;
<IMG>\>       BEGIN INITIAL;
<IMG>.        ;
<IMG>\n       ;

<SRC>\"[^"]+\"     { int i,l; 
                     printf("%s",prefix); 
                     l=strlen(yytext); 
                     for(i=1;i<l-1;i++)
                       printf("%c",yytext[i]);
                     printf(" "); 
                     BEGIN INITIAL;
                   }
<SRC>[^[:space:]]+ printf("%s%s ",prefix, yytext); BEGIN INITIAL;

%%

int main(int argc, char *argv[])
{ if (argc<2) 
  { fprintf(stderr,"Usage: imglist prefix < file.html\n");
    return 1;
  }
  prefix=argv[1];
  yylex();
  return 0;
}