#define mm 0x98
#define lop_quote 0x0
#define lop_loc 0x1
#define lop_skip 0x2
#define lop_fixo 0x3
#define lop_fixr 0x4
#define lop_fixrx 0x5
#define lop_file 0x6
#define lop_line 0x7
#define lop_spec 0x8
#define lop_pre 0x9
#define lop_post 0xa
#define lop_stab 0xb
#define lop_end 0xc \

#define err(m) {fprintf(stderr,"Error in tetra %d: %s!\n",count,m) ;continue;} \
 \

#define y buf[2]
#define z buf[3] \

#define max_image_tetras 0x10000 \

/*1:*/
#line 18 "./mmoboot.w"

#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <string.h> 
/*5:*/
#line 68 "./mmoboot.w"

#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif

/*:5*/
#line 23 "./mmoboot.w"

/*7:*/
#line 96 "./mmoboot.w"

typedef unsigned char byte;
typedef unsigned int tetra;
typedef struct{tetra h,l;}octa;

/*:7*/
#line 24 "./mmoboot.w"

/*4:*/
#line 62 "./mmoboot.w"

int listing;
int verbose;
FILE*mmo_file;
FILE*image_file;

/*:4*//*11:*/
#line 154 "./mmoboot.w"

int count;
int byte_count;
byte buf[4];
int yz;
tetra tet;

/*:11*//*16:*/
#line 197 "./mmoboot.w"

octa cur_loc;
int listed_file;
int cur_file;
int cur_line;
char*file_name[256];
octa tmp;

/*:16*//*24:*/
#line 347 "./mmoboot.w"

tetra image[max_image_tetras];
int higest_image_tetra= 0;

/*:24*/
#line 25 "./mmoboot.w"

/*8:*/
#line 103 "./mmoboot.w"

octa incr ARGS((octa,int));
octa incr(o,delta)
octa o;
int delta;
{
register tetra t;
octa x;
if(delta>=0){
t= 0xffffffff-delta;
if(o.l<=t)x.l= o.l+delta,x.h= o.h;
else x.l= o.l-t-1,x.h= o.h+1;
}else{
t= -delta;
if(o.l>=t)x.l= o.l-t,x.h= o.h;
else x.l= o.l+(0xffffffff+delta)+1,x.h= o.h-1;
}
return x;
}

/*:8*//*9:*/
#line 128 "./mmoboot.w"

void read_tet ARGS((void));
void read_tet()
{
if(fread(buf,1,4,mmo_file)!=4){
fprintf(stderr,"Unexpected end of file after %d tetras!\n",count);

exit(-3);
}
yz= (buf[2]<<8)+buf[3];
tet= (((buf[0]<<8)+buf[1])<<16)+yz;
if(verbose)printf("  %08x\n",tet);
count++;
}

/*:9*//*10:*/
#line 143 "./mmoboot.w"

byte read_byte ARGS((void));
byte read_byte()
{
register byte b;
if(!byte_count)read_tet();
b= buf[byte_count];
byte_count= (byte_count+1)&3;
return b;
}

/*:10*//*25:*/
#line 354 "./mmoboot.w"

void store_image ARGS((octa,tetra));
void store_image(loc,tet)
octa loc;
tetra tet;
{int i;
if(loc.h!=0x80000000)return;
i= loc.l>>2;
if(i>=max_image_tetras)
{fprintf(stderr,"Location %x to large for image (max %x)",loc.l,max_image_tetras*4);
exit(1);
}
image[i]^= tet;
if(i> higest_image_tetra)higest_image_tetra= i;
}

/*:25*/
#line 26 "./mmoboot.w"


int main(argc,argv)
int argc;char*argv[];
{
register int j,delta,postamble= 0;
register char*p;
/*2:*/
#line 41 "./mmoboot.w"

listing= 1,verbose= 0;
for(j= 1;j<argc-1&&argv[j][0]=='-'&&argv[j][2]=='\0';j++){
if(argv[j][1]=='s')listing= 0;
else if(argv[j][1]=='v')verbose= 1;
else break;
}
if(j!=argc-1){
fprintf(stderr,"Usage: %s [-s] [-v] mmofile\n",argv[0]);

exit(-1);
}

/*:2*/
#line 33 "./mmoboot.w"
;
/*3:*/
#line 54 "./mmoboot.w"

mmo_file= fopen(argv[argc-1],"rb");
if(!mmo_file){
fprintf(stderr,"Can't open file %s!\n",argv[argc-1]);

exit(-2);
}

/*:3*//*12:*/
#line 161 "./mmoboot.w"

count= byte_count= 0;

/*:12*//*17:*/
#line 205 "./mmoboot.w"

cur_loc.h= cur_loc.l= 0;
listed_file= cur_file= -1;
cur_line= 0;

/*:17*/
#line 34 "./mmoboot.w"
;
/*23:*/
#line 320 "./mmoboot.w"

read_tet();
if(buf[0]!=mm||buf[1]!=lop_pre){
fprintf(stderr,"Input is not an MMO file (first two bytes are wrong)!\n");

exit(-5);
}
if(y!=1)fprintf(stderr,
"Warning: I'm reading this file as version 1, not version %d!",y);

if(z> 0){
j= z;
read_tet();
if(listing)
printf("File was created %s",asctime(localtime((time_t*)&tet)));
for(j--;j> 0;j--){
read_tet();
if(listing)printf("Preamble data %08x\n",tet);
}
}

/*:23*/
#line 35 "./mmoboot.w"
;
do/*13:*/
#line 166 "./mmoboot.w"

{
read_tet();
loop:if(buf[0]==mm)switch(buf[1]){
case lop_quote:if(yz!=1)
err("YZ field of lop_quote should be 1");

read_tet();break;
/*18:*/
#line 217 "./mmoboot.w"

case lop_loc:if(z==2){
j= y;read_tet();cur_loc.h= (j<<24)+tet;
}else if(z==1)cur_loc.h= y<<24;
else err("Z field of lop_loc should be 1 or 2");

read_tet();cur_loc.l= tet;
continue;
case lop_skip:cur_loc= incr(cur_loc,yz);continue;

/*:18*//*19:*/
#line 231 "./mmoboot.w"

case lop_fixo:if(z==2){
j= y;read_tet();tmp.h= (j<<24)+tet;
}else if(z==1)tmp.h= y<<24;
else err("Z field of lop_fixo should be 1 or 2");

read_tet();tmp.l= tet;
store_image(tmp,cur_loc.h);
tmp= incr(tmp,4);
store_image(tmp,cur_loc.l);
continue;
case lop_fixr:delta= yz;goto fixr;
case lop_fixrx:j= yz;if(j!=16&&j!=24)
err("YZ field of lop_fixrx should be 16 or 24");

read_tet();delta= tet;
if(delta&0xfe000000)err("increment of lop_fixrx is too large");

fixr:tmp= incr(cur_loc,-(delta>=0x1000000?(delta&0xffffff)-(1<<j):delta)<<2);
store_image(tmp,delta);
continue;

/*:19*//*20:*/
#line 255 "./mmoboot.w"

case lop_file:if(file_name[y]){
if(z)err("Two file names with the same number");

for(j= z;j> 0;j--)read_tet();
cur_file= y;
}else{
if(!z)err("No name given for newly selected file");

file_name[y]= (char*)calloc(4*z+1,1);
if(!file_name[y]){
fprintf(stderr,"No room to store the file name!\n");exit(-4);

}
cur_file= y;
for(j= z,p= file_name[y];j> 0;j--,p+= 4){
read_tet();
*p= buf[0];*(p+1)= buf[1];*(p+2)= buf[2];*(p+3)= buf[3];
}
}
cur_line= 0;continue;
case lop_line:if(cur_file<0)err("No file was selected for lop_line");

cur_line= yz;continue;

/*:20*//*21:*/
#line 283 "./mmoboot.w"

case lop_spec:if(listing){
printf("Special data %d at loc %08x%08x",yz,cur_loc.h,cur_loc.l);
if(!cur_line)printf("\n");
else if(cur_file==listed_file)printf(" (line %d)\n",cur_line);
else{
printf(" (\"%s\", line %d)\n",file_name[cur_file],cur_line);
listed_file= cur_file;
}
}
while(1){
read_tet();
if(buf[0]==mm){
if(buf[1]!=lop_quote||yz!=1)goto loop;
read_tet();
}
if(listing)printf("                   %08x\n",tet);
}

/*:21*//*22:*/
#line 304 "./mmoboot.w"

case lop_pre:err("Can't have another preamble");

case lop_post:postamble= 1;
if(y)err("Y field of lop_post should be zero");

if(z<32)err("Z field of lop_post must be 32 or more");

continue;
case lop_stab:err("Symbol table must follow postamble");

case lop_end:err("Symbol table can't end before it begins");

/*:22*/
#line 174 "./mmoboot.w"

default:err("Unknown lopcode");

}
if(listing)/*15:*/
#line 192 "./mmoboot.w"

{store_image(cur_loc,tet);
cur_loc= incr(cur_loc,4);cur_loc.l&= -4;
}

/*:15*/
#line 178 "./mmoboot.w"
;
}

/*:13*/
#line 36 "./mmoboot.w"
while(!postamble);
/*27:*/
#line 394 "./mmoboot.w"

/*26:*/
#line 374 "./mmoboot.w"

{char*image_file_name,*extension;
image_file_name= (char*)calloc(strlen(argv[argc-1])+5,1);
if(!image_file_name){
fprintf(stderr,"No room to store the file name!\n");exit(-4);
}
strcpy(image_file_name,argv[argc-1]);
extension= image_file_name+strlen(image_file_name)-4;
if(strcmp(extension,".mmo")==0||strcmp(extension,".mmo")==0)
strcpy(extension,".img");
else
strcat(image_file_name,".img");
image_file= fopen(image_file_name,"wb");
if(!image_file)
{fprintf(stderr,"Can't open file %s!\n","bios.img");
exit(-3);
}
}

/*:26*/
#line 395 "./mmoboot.w"

{int i;
unsigned char buffer[4];
tetra tet;
for(i= 0;i<=higest_image_tetra;i++)
{tet= image[i];
buffer[0]= (tet>>(3*8))&0xFF;
buffer[1]= (tet>>(2*8))&0xFF;
buffer[2]= (tet>>(1*8))&0xFF;
buffer[3]= (tet)&0xFF;
fwrite(buffer,1,4,image_file);
}
}
fclose(image_file);


/*:27*/
#line 37 "./mmoboot.w"
;
return 0;
}

/*:1*/
