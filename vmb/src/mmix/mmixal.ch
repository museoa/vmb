@x
The switchable type name \&{Char} provides at least a first step
towards a brighter future with Unicode.

@<Type...@>=
typedef unsigned char Char; /* bytes that will become wydes some day */
@y
The switchable type name \&{Char} provides at least a first step
towards a brighter future with Unicode.

@<Type...@>=
typedef char Char; /* bytes that will become wydes some day */
@z




@x
  mmo_loc();
@y
  mmo_loc();

@ In order to prepare a \TeX-output, we define starting and end tags
for all types of output.
@d tex_nl "\\\\ "  //newline
@d tex_tab " & " //"\\> "  // tab
@d tex_startlinenr ""
@d tex_endlinenr " & "
@d tex_nolabel " & "
@d tex_labelstart " "
@d tex_labelend " & "
@d tex_opcodestart ""
@d tex_opcodeend " & "
@d tex_startop ""
@d tex_endop ""
@d tex_startlcomment " & \\relax "//start of a comment following an instruction
@d tex_startcomment " \\comment{\\relax "
@d tex_endcomment "}\\\\ "
@d tex_line_wo_comment "\\hidewidth & "
@d tex_startstring "{\\tt \\symbol{34}"
@d tex_endstring "\\symbol{34}}"
@d tex_blank "\\texttt{\\symbol{32}}"
@d tex_braceopen "{\\tt\\symbol{123}}"
@d tex_braceclose "{\\tt\\symbol{125}}"
@d tex_pipe "\\(|\\)"
@d tex_hat "\\^{ }"
@d tex_ll "\\(\\ll\\)"
@d tex_gg "\\(\\gg\\)"
@d TEX_MAXSYM_LEN 511

@ Furthermore, we need a conversion function to eliminate underscores etc. 
in symbols for putting out \TeX format. (complete?)
Also we trace here, whether we are in string-mode.
@<Global var...@>+=
Char tex_res[TEX_MAXSYM_LEN];
Char *start_comment=NULL;
int string_mode = 0;

@ @<Subroutines@>+=
Char *toTex(Char *symbol)
{ int i, j;
  i=j=0; 
  while(j<TEX_MAXSYM_LEN-1 && symbol[i]){
    tex_res[j] = '\0';
    switch(symbol[i]){
      case '&':
      case '#':
       case '%':  //can occur for comments to ops w/o operands
                 tex_res[j++]='\\'; break;
      case '{': if(strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_braceopen))
                  strcat(tex_res, tex_braceopen);
	        i++, j+=(int)strlen(tex_braceopen);
                continue;
      case '}': if(strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_braceclose))
                  strcat(tex_res, tex_braceclose);
	        i++, j+=(int)strlen(tex_braceclose);
                continue;
      case '|': if(strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_pipe))
                  strcat(tex_res, tex_pipe);
	        i++, j+=(int)strlen(tex_pipe);
                continue;
      case '^': if(strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_hat))
                  strcat(tex_res, tex_hat);
	        i++, j+=(int)strlen(tex_hat);
                continue;
      case '>': if(symbol[i+1]=='>' && strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_gg))
                  strcat(tex_res, tex_gg);
                i+=2, j+=(int)strlen(tex_gg);
                continue;
      case '<': if(symbol[i+1]=='<' && strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_ll))
                  strcat(tex_res, tex_ll);
                i+=2, j+=(int)strlen(tex_ll);
                continue;
      case ' ': if(string_mode && j+strlen(tex_blank) < TEX_MAXSYM_LEN){
                tex_res[j]='\0';
                strcat(tex_res, tex_blank);
                j+=(int)strlen(tex_blank);}
                /* copy blanks to output in case that |toTex| is used for
                preprocessor directives -- DIRTY */
                else break;
                
                i++;
                continue;
      case '\"': 
           tex_res[j]='\0';
           if(string_mode){ 
              if(j+strlen(tex_endstring) < TEX_MAXSYM_LEN) 
                strcat(tex_res, tex_endstring), j+=strlen(tex_endstring);}
           else {if(j+strlen(tex_startstring) < TEX_MAXSYM_LEN) 
                strcat(tex_res, tex_startstring), j+=strlen(tex_startstring);}
           string_mode ^= 1;
           i++;
           continue;
    }
    tex_res[j++] = symbol[i++];
  }
  tex_res[j] = '\0';
  return tex_res;
}
@z

@x
typedef struct sym_tab_struct {
  int serial; /* serial number of symbol; type number for fixups */
  struct sym_tab_struct *link; /* |DEFINED| status or link to fixup */
  octa equiv; /* the equivalent value */
} sym_node;
@y
typedef struct sym_tab_struct {
  int serial; /* serial number of symbol; type number for fixups */
  struct sym_tab_struct *link; /* |DEFINED| status or link to fixup */
  octa equiv; /* the equivalent value */
  int linenr; /* line number for \TeX output */
} sym_node;
@z

@x
  fprintf(listing_file," (%d)\n",pp->serial);
@y
  fprintf(listing_file," (%d)\n",pp->serial);
  if(tex_file) fprintf(tex_file, "%s%s%s: %s%d%s %s\n", tex_startop, toTex(sym_buf+1), 
    tex_endop, tex_startlinenr, 
   pp->linenr, tex_endlinenr, tex_nl);
@z

@x
if (!*p) goto bypass;
@y
if (!*p) {if(tex_file) fprintf(tex_file, "%s%s", tex_nolabel, tex_nl); goto bypass;}
@z

@x
  if (!isdigit(*p)&&!isletter(*p)) goto bypass; /* comment */
@y
  if (!isdigit(*p)&&!isletter(*p)) {/* comment */
     // use p+1 to suppress the comment starting character!
     if(tex_file && *p)
       if(*p=='#') // treat preprocessor commands separately as ops
       fprintf(tex_file, "\\relax %s%s%s\\hidewidth%s", 
            tex_startop, toTex(p), tex_endop, tex_nl);
       else 
       fprintf(tex_file, "%s%s%s\n", tex_startcomment, p, tex_endcomment);
     goto bypass; 
  }
@z

@x
for (p++;isspace(*p);p++);
@y
if(tex_file) 
  fprintf(tex_file, "%s %s %s", tex_labelstart, toTex(lab_field), tex_labelend);
for (p++;isspace(*p);p++);
@z

@x
if (!isspace(*p) && *p && op_field[0]) derr("opcode syntax error at `%c'",*p);
@y
if (!isspace(*p) && *p && op_field[0]) {/* not a nice fix to get macros to TeX */
	if(tex_file && op_field) fprintf(tex_file, "%s%s%s%s", op_field, tex_tab, p, tex_nl);
	derr("opcode syntax error at `%c'",*p);
}
@z

@x
if (!pp) {
@y
if (!pp) {
  if(tex_file) fprintf(tex_file, tex_nl);
@z

@x
opcode=pp->equiv.h, op_bits=pp->equiv.l;
@y
opcode=pp->equiv.h, op_bits=pp->equiv.l;
if(tex_file) fprintf(tex_file, "%s %s %s", tex_opcodestart, toTex(op_field), tex_opcodeend);
@z

@x
else p=""; /* if not followed by semicolon, rest of the line is a comment */
@y
else start_comment=p, p=""; /* if not followed by semicolon, rest of the line is a comment */
@z

@x
*q='\0';
@y
*q='\0';
if(tex_file) fprintf(tex_file, "%s%s%s", tex_startop, toTex(operand_list), tex_endop);
@z

@x
  pp->equiv=cur_loc;@+ pp->link=new_link;
@y
  pp->equiv=cur_loc;@+ pp->link=new_link;@+ pp->linenr=line_no;
@z

@x
\bull\.{-l listingname}\quad Output a listing of the assembled input and
output to a text file called \.{listingname}.
@y
\bull\.{-l listingname}\quad Output a listing of the assembled input and
output to a text file called \.{listingname}.

\bull\.{-t TeXfilename}\quad Output a \TeX formatted version of the assembled 
input and output to a text file called \.{texfilename}.
@z

@x
    @<Get the next line of input text, or |break| if the input has ended@>;
@y
    @<Get the next line of input text, or |break| if the input has ended@>;
   if(tex_file) 
    fprintf(tex_file, "%s%d %s", tex_startlinenr, line_no, tex_endlinenr);
@z

@x
    if (listing_file) {
@y
    if(tex_file && start_comment ){
       if(*start_comment)//|start_comment| might also point to a null character
         fprintf(tex_file, "%s%s%s\n", tex_startlcomment, start_comment, tex_nl);
       else fprintf(tex_file, "%s%s\n", tex_line_wo_comment, tex_nl);
       start_comment=NULL;}
    if (listing_file) {
@z

@x
  else if (argv[j][1]=='o') j++,strcpy(obj_file_name,argv[j]);
@y
  else if (argv[j][1]=='o') j++,strcpy(obj_file_name,argv[j]);
  else if (argv[j][1]=='t') j++,strcpy(tex_file_name,argv[j]);
@z

@x
    argv[0],"[-x] [-l listingname] [-b buffersize] [-o objectfilename]");
@y
    argv[0],"[-x] [-l listingname] [-t TeXfilename] [-b buffersize] [-o objectfilename]");
@z

@x
if (listing_name[0]) {
@y
if (tex_file_name[0]) {
  tex_file=fopen(tex_file_name,"w");
  if (!tex_file) dpanic("Can't open the TeX file %s",tex_file_name); 
}
if (listing_name[0]) {
@z

@x
char listing_name[FILENAME_MAX+1]; /* name of the optional listing file */
FILE *src_file, *obj_file, *listing_file;
@y
char listing_name[FILENAME_MAX+1]; /* name of the optional listing file */
char tex_file_name[FILENAME_MAX+1]; /* name of the \TeX output file */
FILE *src_file, *obj_file, *listing_file, *tex_file;
@z

