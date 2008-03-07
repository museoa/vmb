% This file is part of the MMIXware package (c) Donald E Knuth 1999
@i boilerplate.w %<< legal stuff: PLEASE READ IT BEFORE MAKING ANY CHANGES!

\def\title{MMOBOOT}
\def\MMIX{\.{MMIX}}
\def\MMIXAL{\.{MMIXAL}}
\def\Hex#1{\hbox{$^{\scriptscriptstyle\#}$\tt#1}} % experimental hex constant

@* Introduction. This program reads a binary \.{mmo} file output by
the \MMIXAL\ processor and extracts from it an image file suitable 
as Boot Image for the ROM simulator of the Virtual Motherboard project.
This program is written as a change file to the mmotype program of Donald E. Knuth.
It extracts all data that goes to negative addresses into a large array
of tetras and writes these tetras to the output file after completeing the run.

@s tetra int

@c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
@<Prototype preparations@>@;
@<Type definitions@>@;
@<Global variables@>@;
@<Subroutines@>@;
@#
int main(argc,argv)
  int argc;@+char*argv[];
{
  register int j,delta,postamble=0;
  register char *p;
  @<Process the command line@>;
  @<Initialize everything@>;
  @<List the preamble@>;
  do @<List the next item@>@;@+while (!postamble);
  @<Write the image file@>;
  return 0;
}

@ @<Process the command line@>=
listing=1, verbose=0;
for (j=1;j<argc-1 && argv[j][0]=='-' && argv[j][2]=='\0';j++) {
  if (argv[j][1]=='s') listing=0;
  else if (argv[j][1]=='v') verbose=1;
  else break;
}
if (j!=argc-1) {
  fprintf(stderr,"Usage: %s [-s] [-v] mmofile\n",argv[0]);
@.Usage: ...@>
  exit(-1);
}

@ @<Initialize everything@>=
mmo_file=fopen(argv[argc-1],"rb");
if (!mmo_file) {
  fprintf(stderr,"Can't open file %s!\n",argv[argc-1]);
@.Can't open...@>
  exit(-2);
}

@ @<Glob...@>=
int listing; /* are we listing everything? */
int verbose; /* are we also showing the tetras of input as they are read? */
FILE *mmo_file; /* the input file */
FILE *image_file; /* the output file */

@ @<Prototype preparations@>=
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif

@ A complete definition of \.{mmo} format appears in the \MMIXAL\ document.
Here we need to define only the basic constants used for interpretation.

@d mm 0x98 /* the escape code of \.{mmo} format */
@d lop_quote 0x0 /* the quotation lopcode */
@d lop_loc 0x1 /* the location lopcode */
@d lop_skip 0x2 /* the skip lopcode */
@d lop_fixo 0x3 /* the octabyte-fix lopcode */
@d lop_fixr 0x4 /* the relative-fix lopcode */
@d lop_fixrx 0x5 /* extended relative-fix lopcode */
@d lop_file 0x6 /* the file name lopcode */
@d lop_line 0x7 /* the file position lopcode */
@d lop_spec 0x8 /* the special hook lopcode */
@d lop_pre 0x9 /* the preamble lopcode */
@d lop_post 0xa /* the postamble lopcode */
@d lop_stab 0xb /* the symbol table lopcode */
@d lop_end 0xc /* the end-it-all lopcode */

@* Low-level arithmetic. This program is intended to work correctly
whenever an |int| has at least 32 bits.

@<Type...@>=
typedef unsigned char byte; /* a monobyte */
typedef unsigned int tetra; /* a tetrabyte */
typedef struct {@+tetra h,l;}@+octa; /* an octabyte */

@ The |incr| subroutine adds a signed integer to an (unsigned) octabyte.

@<Sub...@>=
octa incr @,@,@[ARGS((octa,int))@];
octa incr(o,delta)
  octa o;
  int delta;
{
  register tetra t;
  octa x;
  if (delta>=0) {
    t=0xffffffff-delta;
    if (o.l<=t) x.l=o.l+delta, x.h=o.h;
    else x.l=o.l-t-1, x.h=o.h+1;
  } else {
    t=-delta;
    if (o.l>=t) x.l=o.l-t, x.h=o.h;
    else x.l=o.l+(0xffffffff+delta)+1, x.h=o.h-1;
  }
  return x;
}

@* Low-level input. The tetrabytes of an \.{mmo} file are stored in
friendly big-endian fashion, but this program is supposed to work also
on computers that are little-endian. Therefore we read four successive bytes
and pack them into a tetrabyte, instead of reading a single tetrabyte.

@<Sub...@>=
void read_tet @,@,@[ARGS((void))@];
void read_tet()
{
  if (fread(buf,1,4,mmo_file)!=4) {
    fprintf(stderr,"Unexpected end of file after %d tetras!\n",count);
@.Unexpected end of file...@>
    exit(-3);
  }
  yz=(buf[2]<<8)+buf[3];
  tet=(((buf[0]<<8)+buf[1])<<16)+yz;
  if (verbose) printf("  %08x\n",tet);
  count++;
}

@ @<Sub...@>=
byte read_byte @,@,@[ARGS((void))@];
byte read_byte()
{
  register byte b;
  if (!byte_count) read_tet();
  b=buf[byte_count];
  byte_count=(byte_count+1)&3;
  return b;
}

@ @<Glob...@>=
int count; /* the number of tetrabytes we've read */
int byte_count; /* index of the next-to-be-read byte */
byte buf[4]; /* the most recently read bytes */
int yz; /* the two least significant bytes */
tetra tet; /* |buf| bytes packed big-endianwise */

@ @<Init...@>=
count=byte_count=0;

@* The main loop. Now for the bread-and-butter part of this program.

@<List the next item@>=
{
  read_tet();
 loop:@+if (buf[0]==mm) switch (buf[1]) {
   case lop_quote:@+if (yz!=1)
       err("YZ field of lop_quote should be 1");
@.YZ field...should be 1@>
    read_tet();@+break;
   @t\4@>@<Cases for lopcodes in the main loop@>@;
   default: err("Unknown lopcode");
@.Unknown lopcode@>
  }
  if (listing) @<List |tet| as a normal item@>;
}

@ We want to catch all cases where the rules of \.{mmo} format are
not obeyed. The |err| macro ameliorates this somewhat tedious chore.

@d err(m) {@+fprintf(stderr,"Error in tetra %d: %s!\n",count,m);@+ continue;@+}
@.Error in tetra...@>

@ In a normal situation, the newly read tetrabyte is simply supposed
to be loaded into the current location. We list not only the current
location but also the current file position, if |cur_line| is nonzero
and |cur_loc| belongs to segment~0.

@<List |tet| as a normal item@>=
{ store_image(cur_loc,tet);
  cur_loc=incr(cur_loc,4);@+ cur_loc.l &=-4;
}

@ @<Glob...@>=
octa cur_loc; /* the current location */
int listed_file; /* the most recently listed file number */
int cur_file; /* the most recently selected file number */
int cur_line; /* the current position in |cur_file| */
char *file_name[256]; /* file names seen */
octa tmp; /* an octabyte of temporary interest */

@ @<Init...@>=
cur_loc.h=cur_loc.l=0;
listed_file=cur_file=-1;
cur_line=0;

@* The simple lopcodes. We have already implemented |lop_quote|, which
falls through to the normal case after reading an extra tetrabyte.
Now let's consider the other lopcodes in turn.

@d y buf[2] /* the next-to-least significant byte */
@d z buf[3] /* the least significant byte */

@<Cases...@>=
case lop_loc:@+if (z==2) {
   j=y;@+ read_tet();@+ cur_loc.h=(j<<24)+tet;
 }@+else if (z==1) cur_loc.h=y<<24;
 else err("Z field of lop_loc should be 1 or 2");
@:Z field of lop_loc...}\.{Z field of lop\_loc...@>
 read_tet();@+ cur_loc.l=tet;
 continue;
case lop_skip: cur_loc=incr(cur_loc,yz);@+continue;

@ Fixups load information out of order, when future references have
been resolved. The current file name and line number are not considered
relevant.

@<Cases...@>=
case lop_fixo:@+if (z==2) {
   j=y;@+ read_tet();@+ tmp.h=(j<<24)+tet;
 }@+else if (z==1) tmp.h=y<<24;
 else err("Z field of lop_fixo should be 1 or 2");
@:Z field of lop_fixo...}\.{Z field of lop\_fixo...@>
 read_tet();@+ tmp.l=tet;
 store_image(tmp, cur_loc.h);
 tmp=incr(tmp,4);
 store_image(tmp, cur_loc.l);
 continue;
case lop_fixr: delta=yz; goto fixr;
case lop_fixrx:j=yz;@+if (j!=16 && j!=24)
    err("YZ field of lop_fixrx should be 16 or 24");
@:YZ field of lop_fixrx...}\.{YZ field of lop\_fixrx...@>
 read_tet(); delta=tet;
 if (delta&0xfe000000) err("increment of lop_fixrx is too large");
@.increment...too large@>
fixr: tmp=incr(cur_loc,-(delta>=0x1000000? (delta&0xffffff)-(1<<j): delta)<<2);
 store_image(tmp,delta);
 continue;

@ The space for file names isn't allocated until we are sure we need it.

@<Cases...@>=
case lop_file:@+if (file_name[y]) {
   if (z) err("Two file names with the same number");
@.Two file names...@>
   for (j=z;j>0;j--) read_tet();
   cur_file=y;
 }@+else {
   if (!z) err("No name given for newly selected file");
@.No name given...@>
   file_name[y]=(char*)calloc(4*z+1,1);
   if (!file_name[y]) {
     fprintf(stderr,"No room to store the file name!\n");@+exit(-4);
@.No room...@>
   }
   cur_file=y;
   for (j=z,p=file_name[y]; j>0; j--,p+=4) {
     read_tet();
     *p=buf[0];@+*(p+1)=buf[1];@+*(p+2)=buf[2];@+*(p+3)=buf[3];
   }
 }
 cur_line=0;@+continue;
case lop_line:@+if (cur_file<0) err("No file was selected for lop_line");
@.No file was selected...@>
 cur_line=yz;@+continue;

@ Special bytes in the file might be in synch with the current location
and/or the current file position, so we list those parameters too.

@<Cases...@>=
case lop_spec:@+if (listing) {
   printf("Special data %d at loc %08x%08x", yz, cur_loc.h, cur_loc.l);
   if (!cur_line) printf("\n");
   else if (cur_file==listed_file) printf(" (line %d)\n",cur_line);
   else {
     printf(" (\"%s\", line %d)\n", file_name[cur_file], cur_line);
     listed_file=cur_file;
   }
 }
 while(1) {
   read_tet();
   if (buf[0]==mm) {
     if (buf[1]!=lop_quote || yz!=1) goto loop; /* end of special data */
     read_tet();
   }
   if (listing) printf("                   %08x\n",tet);
 }

@ The other cases shouldn't appear in the main loop.

@<Cases...@>=
case lop_pre: err("Can't have another preamble");
@.Can't have another...@>
case lop_post: postamble=1;
 if (y) err("Y field of lop_post should be zero");
@:Y field of lop_post...}\.{Y field of lop\_post...@>
 if (z<32) err("Z field of lop_post must be 32 or more");
@:Z field of lop_post...}\.{Z field of lop\_post...@>
 continue;
case lop_stab: err("Symbol table must follow postamble");
@.Symbol table...@>
case lop_end: err("Symbol table can't end before it begins");

@* The preamble and postamble. Now here's what we do before and after
the main loop.

@<List the preamble@>=
read_tet(); /* read the first tetrabyte of input */
if (buf[0]!=mm || buf[1]!=lop_pre) {
  fprintf(stderr,"Input is not an MMO file (first two bytes are wrong)!\n");
@.Input is not...@>
  exit(-5);
}
if (y!=1) fprintf(stderr,
    "Warning: I'm reading this file as version 1, not version %d!",y);
@.I'm reading this file...@>
if (z>0) {
  j=z;
  read_tet();
  if (listing)
    printf("File was created %s",asctime(localtime((time_t*)&tet)));
  for (j--;j>0;j--) {
    read_tet();
    if (listing) printf("Preamble data %08x\n",tet);
  }
}

@* Writing the image file.
We first write the image to a long array of tetras keepting track
of the highest index we used in writing.

@d max_image_tetras 0x10000

@<Glob...@>=
tetra image[max_image_tetras];
int higest_image_tetra = 0;

@ We fill the array using this function. It checks that the 
location is negative and will fit into the image.

@<Sub...@>=
void store_image @,@,@[ARGS((octa, tetra))@];
void store_image(loc,tet)
octa loc;
tetra tet;
{ int i;
  if (loc.h!=0x80000000) return;
  i = loc.l>>2;
  if (i>=max_image_tetras) 
  { fprintf(stderr,"Location %x to large for image (max %x)",loc.l, max_image_tetras*4);
    exit(1);
  }
  image[i] ^= tet;
  if (i> higest_image_tetra)higest_image_tetra=i;
}

@ Before we can open the otput file, we have to determine a filename for the output file.
  We either replace the extension .mmo or .MMO of the input file name by .img (for image) or
  we append the extension .img to the input file name.

@<Open the image file@>=
  { char *image_file_name, *extension;
    image_file_name = (char*)calloc(strlen(argv[argc-1])+5,1);
    if (!image_file_name) {
      fprintf(stderr,"No room to store the file name!\n");@+exit(-4);
    }
    strcpy(image_file_name,argv[argc-1]);
    extension = image_file_name+strlen(image_file_name)-4;
    if (strcmp(extension,".mmo")==0 || strcmp(extension,".mmo")==0)
      strcpy(extension,".img");
    else
      strcat(image_file_name,".img");
    image_file=fopen(image_file_name,"wb");
    if (!image_file) 
    { fprintf(stderr,"Can't open file %s!\n","bios.img");
      exit(-3);
    }
  }

@ Last not least we can
@<Write the image file@>=
  @<Open the image file@>
  { int i;
    unsigned char buffer[4];
    tetra tet;
    for (i=0;i<=higest_image_tetra;i++)  
    { tet = image[i]; 
      buffer[0] = (tet>>(3*8))&0xFF;
      buffer[1] = (tet>>(2*8))&0xFF;
      buffer[2] = (tet>>(1*8))&0xFF;
      buffer[3] = (tet)&0xFF;
      fwrite(buffer,1,4,image_file);
    }
  }
  fclose(image_file);


@* Index.
