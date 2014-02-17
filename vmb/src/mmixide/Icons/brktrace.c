/* GIMP RGBA C-Source image dump (brktrace.c) */

static const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[16 * 16 * 4 + 1];
} brktrace = {
  16, 16, 4,
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\307\0\0\0\300\0\0\0\250\0\0\0\222\0\0\0\0\0o\0"
  "\0\0\277\0\0\0\277\0\0\0o\0\0\0\17\301\0\0\0\251\0\0\0\223\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\327\0\0/\360/\0-\364-\0\0\325\0\0\4\4\4\317333\377888"
  "\377\0\0\0\377\0\0\0\377\0\0\0\317\0\0\0\17\327\0\0\0\251\0\0\0\210\0\0\0"
  "\0\0\0\0\0\310\0\0""1\3601\0{\377{\0""4\3724\0\0\0\0o777\377zzz\377333\377"
  "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\177\344\0\0\0\302\0\0\0\222\0\0\0\205"
  "\0\0\0\0\303\0\0<\367<\0""8\3728\0\0\346\0\0\0\0\0\277===\377666\377\0\0"
  "\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\277\327\0\0\0\302\0\0\0\231\0"
  "\0\0\207\0\0\0\0\260\0\0\0\334\0\0\0\347\0\0\0\330\0\0\0\0\0\277\1\1\1\377"
  "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\277\316\0\0\0\267"
  "\0\0\0\221\0\0\0\205\0\0\0\0\226\0\0\0\255\0\0\0\304\0\0\0\303\0\0\0\0\0"
  "o\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\177\267"
  "\0\0\0\236\0\0\0\206\0\0\0\205\0\0\0\0\0\0\0\0\211\0\0\0\224\0\0\0\231\0"
  "\0\0\217\0\0\0\0\0\317\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\317"
  "\0\0\0\17\221\0\0\0\206\0\0\0\205\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\205\0\0"
  "\0\205\0\0\0\205\0\0\0\0\0\17\0\0\0o\0\0\0\277\0\0\0\277\0\0\0\177\0\0\0"
  "\17\205\0\0\0\205\0\0\0\205\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
};
