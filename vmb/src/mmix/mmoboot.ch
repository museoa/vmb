I have to make this diff file into a change file for mmotype.w


*** mmotype.w	2008-02-20 16:35:49.000000000 +0100
--- mmoboot.w	2008-02-24 20:44:23.000000000 +0100
***************
*** 3,5 ****
  
! \def\title{MMOTYPE}
  \def\MMIX{\.{MMIX}}
--- 3,5 ----
  
! \def\title{MMOBOOT}
  \def\MMIX{\.{MMIX}}
***************
*** 9,13 ****
  @* Introduction. This program reads a binary \.{mmo} file output by
! the \MMIXAL\ processor and lists it in human-readable form. It lists
! only the symbol table, if invoked with the \.{-s} option. It lists
! also the tetrabytes of input, if invoked with the \.{-v} option.
  
--- 9,15 ----
  @* Introduction. This program reads a binary \.{mmo} file output by
! the \MMIXAL\ processor and extracts from it an image file suitable 
! as Boot Image for the ROM simulator of the Virtual Motherboard project.
! This program is written as a change file to the mmotype program of Donald E. Knuth.
! It extracts all data that goes to negative addresses into a large array
! of tetras and writes these tetras to the output file after completeing the run.
  
***************
*** 34,37 ****
    do @<List the next item@>@;@+while (!postamble);
!   @<List the postamble@>;
!   @<List the symbol table@>;
    return 0;
--- 36,38 ----
    do @<List the next item@>@;@+while (!postamble);
!   @<Write the image file@>;
    return 0;
***************
*** 64,65 ****
--- 65,67 ----
  FILE *mmo_file; /* the input file */
+ FILE *image_file; /* the output file */
  
***************
*** 190,205 ****
  @<List |tet| as a normal item@>=
! {
!   printf("%08x%08x: %08x",cur_loc.h,cur_loc.l,tet);
!   if (!cur_line) printf("\n");
!   else {
!     if (cur_loc.h&0xe0000000) printf("\n");
!     else {
!       if (cur_file==listed_file) printf(" (line %d)\n",cur_line);
!       else {
!         printf(" (\"%s\", line %d)\n", file_name[cur_file], cur_line);
!         listed_file=cur_file;
!       }
!     }
!     cur_line++;
!   }
    cur_loc=incr(cur_loc,4);@+ cur_loc.l &=-4;
--- 192,194 ----
  @<List |tet| as a normal item@>=
! { store_image(cur_loc,tet);
    cur_loc=incr(cur_loc,4);@+ cur_loc.l &=-4;
***************
*** 248,250 ****
   read_tet();@+ tmp.l=tet;
!  if (listing) printf("%08x%08x: %08x%08x\n",tmp.h,tmp.l,cur_loc.h,cur_loc.l);
   continue;
--- 237,241 ----
   read_tet();@+ tmp.l=tet;
!  store_image(tmp, cur_loc.h);
!  tmp=incr(tmp,4);
!  store_image(tmp, cur_loc.l);
   continue;
***************
*** 258,260 ****
  fixr: tmp=incr(cur_loc,-(delta>=0x1000000? (delta&0xffffff)-(1<<j): delta)<<2);
!  if (listing) printf("%08x%08x: %08x\n",tmp.h,tmp.l,delta);
   continue;
--- 249,251 ----
  fixr: tmp=incr(cur_loc,-(delta>=0x1000000? (delta&0xffffff)-(1<<j): delta)<<2);
!  store_image(tmp,delta);
   continue;
***************
*** 349,464 ****
  
! @ @<List the postamble@>=
! for (j=z;j<256;j++) {
!   read_tet();@+tmp.h=tet;@+read_tet();
!   if (listing) {
!     if (tmp.h || tet) printf("g%03d: %08x%08x\n",j,tmp.h,tet);
!     else printf("g%03d: 0\n",j);
!   }
! }
  
! @* The symbol table. Finally we come to the symbol table, which is
! the most interesting part of this program because it recursively
! traces an implicit ternary trie structure.
! 
! @<List the symbol table@>=
! read_tet();
! if (buf[0]!=mm || buf[1]!=lop_stab) {
!   fprintf(stderr,"Symbol table does not follow the postamble!\n");
! @.Symbol table...@>
!   exit(-6);
! }
! if (yz) fprintf(stderr,"YZ field of lop_stab should be zero!\n");
! @.YZ field...should be zero@>
! printf("Symbol table (beginning at tetra %d):\n",count);
! stab_start=count;
! sym_ptr=sym_buf;
! print_stab();
! @<Check the |lop_end|@>;
! 
! @ The main work is done by a recursive subroutine called |print_stab|,
! which manipulates a global array |sym_buf| containing the current
! symbol prefix; the global variable |sym_ptr| points to the first
! unfilled character of that array.
  
! @<Sub...@>=
! void print_stab @,@,@[ARGS((void))@];
! void print_stab()
! {
!   register int m=read_byte(); /* the master control byte */
!   register int c; /* the character at the current trie node */
!   register int j,k;
!   if (m&0x40) print_stab(); /* traverse the left subtrie, if it is nonempty */
!   if (m&0x2f) {
!     @<Read the character |c|@>;
!     *sym_ptr++=c;
!     if (sym_ptr==&sym_buf[sym_length_max]) {
!       fprintf(stderr,"Oops, the symbol is too long!\n");@+exit(-7);
! @.Oops...too long@>
!     }
!     if (m&0xf)
!       @<Print the current symbol with its equivalent and serial number@>;
!     if (m&0x20) print_stab(); /* traverse the middle subtrie */
!     sym_ptr--;
!   }
!   if (m&0x10) print_stab(); /* traverse the right subtrie, if it is nonempty */
! }
  
! @ The present implementation doesn't support Unicode; characters with
! more than 8-bit codes are printed as `\.?'. However, the changes
! for 16-bit codes would be quite easy if proper fonts for Unicode output
! were available. In that case, |sym_buf| would be an array of wyde characters.
! @^Unicode@>
! @^system dependencies@>
! 
! @<Read the character |c|@>=
! if (m&0x80) j=read_byte(); /* 16-bit character */
! else j=0;  
! c=read_byte();
! if (j) c='?'; /* oops, we can't print |(j<<8)+c| easily at this time */
  
! @ @<Print the current symbol with its equivalent and serial number@>=
! {
!   *sym_ptr='\0';
!   j=m&0xf;
!   if (j==15) sprintf(equiv_buf,"$%03d",read_byte());
!   else if (j<=8) {
!     strcpy(equiv_buf,"#");
!     for (;j>0;j--) sprintf(equiv_buf+strlen(equiv_buf),"%02x",read_byte());
!     if (strcmp(equiv_buf,"#0000")==0) strcpy(equiv_buf,"?"); /* undefined */
!   }@+else {
!     strncpy(equiv_buf,"#20000000000000",33-2*j);
!     equiv_buf[33-2*j]='\0';
!     for (;j>8;j--) sprintf(equiv_buf+strlen(equiv_buf),"%02x",read_byte());
    }
!   for (j=k=read_byte();; k=read_byte(),j=(j<<7)+k) if (k>=128) break;
!     /* the serial number is now $j-128$ */
!   printf("    %s = %s (%d)\n",sym_buf+1,equiv_buf,j-128);
  }
  
! @ @d sym_length_max 1000
  
! @<Glob...@>=
! int stab_start; /* where the symbol table began */
! char sym_buf[sym_length_max];
!    /* the characters on middle transitions to current node */
! char *sym_ptr; /* the character in |sym_buf| following the current prefix */
! char equiv_buf[20]; /* equivalent of the current symbol */
! 
! @ @<Check the |lop_end|@>=
! while (byte_count)
!   if (read_byte()) fprintf(stderr,"Nonzero byte follows the symbol table!\n");
! @.Nonzero byte follows...@>
! read_tet();
! if (buf[0]!=mm || buf[1]!=lop_end)
!   fprintf(stderr,"The symbol table isn't followed by lop_end!\n");
! @.The symbol table isn't...@>
! else if (count!=stab_start+yz+1)
!   fprintf(stderr,"YZ field at lop_end should have been %d!\n",count-yz-1);
! @:YZ field at lop_end...}\.{YZ field at lop\_end...@>
! else {
!   if (verbose) printf("Symbol table ends at tetra %d.\n",count);
!   if (fread(buf,1,1,mmo_file))
!     fprintf(stderr,"Extra bytes follow the lop_end!\n");
! @.Extra bytes follow...@>
! }
  
--- 340,409 ----
  
! @* Writing the image file.
! We first write the image to a long array of tetras keepting track
! of the highest index we used in writing.
  
! @d max_image_tetras 0x10000
  
! @<Glob...@>=
! tetra image[max_image_tetras];
! int higest_image_tetra = 0;
  
! @ We fill the array using this function. It checks that the 
! location is negative and will fit into the image.
  
! @<Sub...@>=
! void store_image @,@,@[ARGS((octa, tetra))@];
! void store_image(loc,tet)
! octa loc;
! tetra tet;
! { int i;
!   if (loc.h!=0x80000000) return;
!   i = loc.l>>2;
!   if (i>=max_image_tetras) 
!   { fprintf(stderr,"Location %x to large for image (max %x)",loc.l, max_image_tetras*4);
!     exit(1);
    }
!   image[i] ^= tet;
!   if (i> higest_image_tetra)higest_image_tetra=i;
  }
  
! @ Before we can open the otput file, we have to determine a filename for the output file.
!   We either replace the extension .mmo or .MMO of the input file name by .img (for image) or
!   we append the extension .img to the input file name.
! 
! @<Open the image file@>=
!   { char *image_file_name, *extension;
!     image_file_name = (char*)calloc(strlen(argv[argc-1])+5,1);
!     if (!image_file_name) {
!       fprintf(stderr,"No room to store the file name!\n");@+exit(-4);
!     }
!     strcpy(image_file_name,argv[argc-1]);
!     extension = image_file_name+strlen(image_file_name)-4;
!     if (strcmp(extension,".mmo")==0 || strcmp(extension,".mmo")==0)
!       strcpy(extension,".img");
!     else
!       strcat(image_file_name,".img");
!     image_file=fopen(image_file_name,"wb");
!     if (!image_file) 
!     { fprintf(stderr,"Can't open file %s!\n","bios.img");
!       exit(-3);
!     }
!   }
  
! @ Last not least we can
! @<write the image file@>=
!   @<Open the image file@>
!   { int i;
!     unsigned char buffer[4];
!     tetra tet;
!     for (i=0;i<=higest_image_tetra;i++)  
!     { tet = image[i]; 
!       buffer[0] = (tet>>(3*8))&0xFF;
!       buffer[1] = (tet>>(2*8))&0xFF;
!       buffer[2] = (tet>>(1*8))&0xFF;
!       buffer[3] = (tet)&0xFF;
!       fwrite(buffer,1,4,image_file);
!     }
!   }
!   fclose(image_file);
  
