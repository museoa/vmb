/*10:*/
#line 1411 "mp32pcm.w"

#include <stdlib.h> 
/*15:*/
#line 1548 "mp32pcm.w"

#define MP3_MIN_BUFFER (2*1152)
/*:15*//*26:*/
#line 1722 "mp32pcm.w"

#define MP3_CONTINUE 0
#define MP3_SKIP   0x0100
#define MP3_REPEAT 0x0200
#define MP3_MUTE   0x0400
#define MP3_REPAIR 0x0800
/*:26*//*27:*/
#line 1737 "mp32pcm.w"

#define MP3_BREAK 0x1000
/*:27*//*28:*/
#line 1752 "mp32pcm.w"

#define MP3_ERROR -1
/*:28*//*40:*/
#line 2095 "mp32pcm.w"

#define MP3_ERROR_NO_INPUT -2
/*:40*//*45:*/
#line 2122 "mp32pcm.w"

#define MP3_ERROR_TOO_MANY -3

/*:45*//*47:*/
#line 2134 "mp32pcm.w"

#define MP3_ERROR_MEMORY -4
/*:47*//*54:*/
#line 2181 "mp32pcm.w"

#define MP3_ERROR_NO_ID -5
#define MP3_ERROR_NOT_OPEN -6
/*:54*//*57:*/
#line 2217 "mp32pcm.w"

#define MP3_ERROR_DONE -7
#define MP3_ERROR_NO_BUFFER -8
#define MP3_ERROR_NO_SIZE -9
/*:57*//*114:*/
#line 3169 "mp32pcm.w"

#define SCALEFACTOR_ERROR 0x0010
/*:114*//*447:*/
#line 12486 "mp32pcm.w"

#define MP3_EQ_UNITGAIN 210
/*:447*/
#line 1413 "mp32pcm.w"

/*14:*/
#line 1535 "mp32pcm.w"

typedef short int mp3_sample;
/*:14*/
#line 1414 "mp32pcm.w"

/*23:*/
#line 1695 "mp32pcm.w"

typedef struct mp3_info{
int id;
unsigned int header;
/*31:*/
#line 1858 "mp32pcm.w"

int version;
#define MP3_V1_0 0x00   
#define MP3_V2_0 0x01   
#define MP3_V2_5 0x02   
int layer;
/*:31*//*32:*/
#line 1867 "mp32pcm.w"

int crc_protected;
int bit_rate;
int frame_size;
int frame_position;
int samples;
int private;
int mode;
#define MP3_STEREO       0x00
#define MP3_JOINT_STEREO 0x01
#define MP3_DUAL_CHANNEL 0x02
#define MP3_MONO         0x03
int copyright;
int original;
int emphasis;
int frame;
/*:32*//*33:*/
#line 1893 "mp32pcm.w"

int sample_rate;
int channels;
int bit_per_sample;
/*:33*//*84:*/
#line 2731 "mp32pcm.w"

int free_format;
/*:84*//*86:*/
#line 2761 "mp32pcm.w"

int frequency_index;
/*:86*//*89:*/
#line 2798 "mp32pcm.w"

int padding;
/*:89*//*96:*/
#line 2891 "mp32pcm.w"

int bound;
/*:96*//*183:*/
#line 4371 "mp32pcm.w"

int changes;
/*:183*//*283:*/
#line 8325 "mp32pcm.w"

int ms_stereo;
int i_stereo;
/*:283*//*432:*/
#line 11834 "mp32pcm.w"

int fixed_size;
/*:432*/
#line 1699 "mp32pcm.w"

}mp3_info;
/*:23*/
#line 1415 "mp32pcm.w"

/*19:*/
#line 1619 "mp32pcm.w"

typedef struct mp3_options{
/*20:*/
#line 1626 "mp32pcm.w"

int flags;
/*:20*//*21:*/
#line 1640 "mp32pcm.w"

#define MP3_TWO_CHANNEL_MONO 0x0100
/*:21*//*22:*/
#line 1686 "mp32pcm.w"

int(*info_callback)(mp3_info*p);
/*:22*//*29:*/
#line 1763 "mp32pcm.w"

#define MP3_INFO_NEVER    0x00 
#define MP3_INFO_IGNORE   0x01 
#define MP3_INFO_ONCE     0x02 
#define MP3_INFO_FRAME    0x04 
#define MP3_INFO_READ     0x08 
#define MP3_INFO_PCM      0x10 
#define MP3_INFO_MPG      0x20 
#define MP3_INFO_CRC      0x40 
#define MP3_INFO_RESERVED 0x80  
/*:29*//*35:*/
#line 1947 "mp32pcm.w"

#define MP3_SYNC_1         0x0400 
#define MP3_SYNC_2         0x0000 
#define MP3_SYNC_3         0x0800 
/*:35*//*36:*/
#line 1966 "mp32pcm.w"

/*37:*/
#line 1974 "mp32pcm.w"

void(*tag_handler)(int id,/*38:*/
#line 1982 "mp32pcm.w"

int tag_read(int id,void*buffer,int count)
/*:38*/
#line 1975 "mp32pcm.w"
)
/*:37*/
#line 1967 "mp32pcm.w"
;
/*:36*//*73:*/
#line 2499 "mp32pcm.w"

#define MP3_DONT_FLUSH 0x0200
/*:73*//*331:*/
#line 9564 "mp32pcm.w"

#define MP3_NO_PARTIAL_FRAME 0x1000
/*:331*//*445:*/
#line 12456 "mp32pcm.w"

unsigned char(*equalizer)[32];
/*:445*/
#line 1621 "mp32pcm.w"

}mp3_options;
/*:19*/
#line 1416 "mp32pcm.w"

/*11:*/
#line 1470 "mp32pcm.w"

extern int mp3_open(
/*12:*/
#line 1489 "mp32pcm.w"

int(*input_read)(int id,void*buffer,size_t size)
/*:12*/
#line 1472 "mp32pcm.w"
,
/*18:*/
#line 1610 "mp32pcm.w"

mp3_options*option_pointer
/*:18*/
#line 1473 "mp32pcm.w"
)
/*:11*/
#line 1417 "mp32pcm.w"
;
/*13:*/
#line 1519 "mp32pcm.w"

extern int mp3_read(int id,mp3_sample*buffer,int size)
/*:13*/
#line 1418 "mp32pcm.w"
;
/*16:*/
#line 1568 "mp32pcm.w"

extern int mp3_close(int id)
/*:16*/
#line 1419 "mp32pcm.w"
;
/*:10*/
