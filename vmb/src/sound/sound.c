/*
 * sound.c -- sound simulation
   The following code is thanks to the gimmix project.
   It was ported to fitt the mmix motherboard by Martin Ruckert. 2005

 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma warning(disable : 4996)

#include "winopt.h"
#include "resource.h"
extern HWND hMainWnd;
#include "inspect.h"
#include "vmb.h"
#include "error.h"
#include "bus-arith.h"
#include "param.h"

int major_version=1, minor_version=8;
extern device_info vmb;

char title[] ="VMB Sound";

char version[]="$Revision: 1.1 $ $Date: 2015-10-05 11:39:02 $";

char howto[] =
"This simulates a sound card, see the HTML Help.\n";

#define NUM_REGS 41
struct register_def sound_regs[NUM_REGS+1] = {
	/* name offset size chunk format */
	{"Control"  ,0x00,1,byte_chunk,unsigned_format},
	{"Format"	,0x01,1,byte_chunk,hex_format},
	{"Playing"  ,0x02,1,byte_chunk,unsigned_format},
	{"Finished" ,0x03,1,byte_chunk,unsigned_format},
	{"Buffer 0" ,0x04,1,byte_chunk,unsigned_format},
	{"Buffer 1" ,0x05,1,byte_chunk,unsigned_format},
	{"Buffer 2" ,0x06,1,byte_chunk,unsigned_format},
	{"Buffer 3" ,0x07,1,byte_chunk,unsigned_format},
	{"Position" ,0x08,8,octa_chunk,unsigned_format},
    {"1 Addr",0x10,8,octa_chunk,hex_format},
    {"1 Size",0x18,8,octa_chunk,unsigned_format},
    {"2 Addr",0x20,8,octa_chunk,hex_format},
    {"2 Size",0x28,8,octa_chunk,unsigned_format},
    {"3 Addr",0x30,8,octa_chunk,hex_format},
    {"3 Size",0x38,8,octa_chunk,unsigned_format},
    {"4 Addr",0x40,8,octa_chunk,hex_format},
    {"4 Size",0x48,8,octa_chunk,unsigned_format},
    {"5 Addr",0x50,8,octa_chunk,hex_format},
    {"5 Size",0x58,8,octa_chunk,unsigned_format},
    {"6 Addr",0x60,8,octa_chunk,hex_format},
    {"6 Size",0x68,8,octa_chunk,unsigned_format},
    {"7 Addr",0x70,8,octa_chunk,hex_format},
    {"7 Size",0x78,8,octa_chunk,unsigned_format},
    {"8 Addr",0x80,8,octa_chunk,hex_format},
    {"8 Size",0x88,8,octa_chunk,unsigned_format},
    {"9 Addr",0x90,8,octa_chunk,hex_format},
    {"9 Size",0x98,8,octa_chunk,unsigned_format},
    {"10 Addr",0xa0,8,octa_chunk,hex_format},
    {"10 Size",0xa8,8,octa_chunk,unsigned_format},
    {"11 Addr",0xb0,8,octa_chunk,hex_format},
    {"11 Size",0xb8,8,octa_chunk,unsigned_format},
    {"12 Addr",0xc0,8,octa_chunk,hex_format},
    {"12 Size",0xc8,8,octa_chunk,unsigned_format},
    {"13 Addr",0xd0,8,octa_chunk,hex_format},
    {"13 Size",0xd8,8,octa_chunk,unsigned_format},
    {"14 Addr",0xe0,8,octa_chunk,hex_format},
    {"14 Size",0xe8,8,octa_chunk,unsigned_format},
    {"15 Addr",0xf0,8,octa_chunk,hex_format},
    {"15 Size",0xf8,8,octa_chunk,unsigned_format},
    {"16 Addr",0x100,8,octa_chunk,hex_format},
    {"16 Size",0x108,8,octa_chunk,unsigned_format},
	{0}};

#define SOUND_MEM 0x110
static unsigned char mem[SOUND_MEM] = {0};

#define SECTOR_SIZE	0x800	/* sector size in bytes */

#define SOUND_CTRL_OFFSET		0x00			
#define SOUND_CTRL				(mem[SOUND_CTRL_OFFSET])	    /* control register */
#define SOUND_FORMAT_OFFSET		0x01
#define SOUND_FORMAT			(mem[SOUND_FORMAT_OFFSET])	/* format register */
#define SOUND_PLAYING			(mem[2])
#define SOUND_FINISHED			(mem[3])
#define SOUND_BUF0				(mem[4])
#define SOUND_BUF1				(mem[5])
#define SOUND_BUF2				(mem[6])
#define SOUND_BUF3				(mem[7])
#define SOUND_BUFFERS			GET4(mem+0x4)  /* all four buffers */
#define SOUND_POS				GET8(mem+0x08) /* sound position */

#define SOUND_DMA_OFFSET		0x10
#define SOUND_DMA_ADDR(i)		GET8(mem+SOUND_DMA_OFFSET+i*0x10)	/* DMA address register */
#define SOUND_DMA_SIZE(i)		GET8(mem+SOUND_DMA_OFFSET+8+i*0x10)	/* DMA size register */

#define SET_SOUND_PLAYING(x)		(SOUND_PLAYING=(x))       /* playing register */
#define SET_SOUND_FINISHED(x)		(SOUND_FINISHED=(x))	    /* finished register */
#define SET_SOUND_POS(x)			SET8(mem+0x08,x)	/* sound position register */
#define SET_SOUND_DMA_ADDR(i,x)		SET8(mem+SOUND_DMA_OFFSET+i*0x10,x)	/* DMA address register */
#define SET_SOUND_DMA_SIZE(i,x)		SET8(mem+SOUND_DMA_OFFSET+8+i*0x10,x)	/* DMA size register */

/* commands Bits */
#define SOUND_IGNORE		0
#define SOUND_PLAYONCE		1
#define SOUND_PLAYLOOP		2
#define SOUND_PRELOAD		3
#define SOUND_UNLOAD		4
#define SOUND_INTERRUPT  	0x10	/* a 1 written here to enable interrupts*/
#define SOUND_CANCEL		0x80
#define SOUND_RESET			0xFF


/* copies of read only registers */
DWORD dwSoundThreadId;
static uint64_t position=0; /* owned by player thread */
static unsigned char playing; /* owned by player thread */
static unsigned char finished; /* owned by player thread */
/* copies of registers that are never changed by the player thread */
static struct {
	uint64_t address; /* owned by vmb thread, read only for player thread */
	uint64_t size; /* owned by vmb thread, read only for player thread */
	uint64_t loaded; /* owned by player thread, buffer if existing is loaded from 0 to loaded-1 */
	uint64_t allocated; /* owned by player thread */
	uint64_t played; /* owned by player thread, is played from 0 to played-1  */
	unsigned char *buffer; /* owned by player thread, if not NULL has allocated size */
}	soundDma[16];

/* the sound dma server */
/* messages to the sound dma server thread */
#define WM_SOUND_IGNORE				(WM_APP+1)
#define WM_SOUND_PRELOAD			(WM_APP+2)
#define WM_SOUND_CANCEL				(WM_APP+3)
#define WM_SOUND_RESET				(WM_APP+4)
#define WM_SOUND_TERMINATE			(WM_APP+5)
#define WM_SOUND_PLAYONCE			(WM_APP+6)
#define WM_SOUND_PLAYLOOP			(WM_APP+7)
#define WM_SOUND_UNLOAD			    (WM_APP+8)





/* Utilities to manage the virtual registers */

static void mem_to_register(int offset, int size)
/* moving mem data to read only registers */
{  int i;
   if (offset<SOUND_DMA_OFFSET) return;
for (i=(offset-SOUND_DMA_OFFSET)/0x10;i<16 && i*0x10+SOUND_DMA_OFFSET<offset+size;i++)
   { soundDma[i].address=SOUND_DMA_ADDR(i);
     soundDma[i].size=SOUND_DMA_SIZE(i);
     PostThreadMessage(dwSoundThreadId, WM_SOUND_UNLOAD,0,i+1); 
   }
}


static void register_to_mem(int offset, int size)
/* moving register data to read only memory */
{ SET_SOUND_PLAYING(playing);
  SET_SOUND_FINISHED(finished);
  SET_SOUND_POS(position);
}

/* connecting to the virtual motherbaord */

int sound_reg_read(unsigned int offset, int size, unsigned char *buf)
{ if (offset>SOUND_MEM) return 0;
  if (offset+size>SOUND_MEM) size =SOUND_MEM-offset;
  register_to_mem(offset,size);
  memmove(buf,mem+offset,size);
  return size;
}

unsigned char *sound_get_payload(unsigned int offset, int size)
     /* read an octabyte  (one of the five registers)*/
{  
   register_to_mem(offset,size);	
   return mem+offset;
}

static void set_soundCtrl(void);
static void soundInit(void);
static void soundExit(void);

void sound_put_payload(unsigned int offset, int size, unsigned char *payload)
{  memmove(mem+offset,payload,size);
   mem_update(offset,size);
   mem_to_register(offset,size);
   if (offset==0 && size>0)
      set_soundCtrl();
}


void sound_poweron(void)
{  soundInit();
   PostMessage(hMainWnd,WM_VMB_ON,0,0);
}


void sound_poweroff(void)
{ soundExit();
  PostMessage(hMainWnd,WM_VMB_OFF,0,0);
}


void sound_reset(void)
{ soundExit();
  soundInit();
}

void sound_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ soundExit();
  PostMessage(hMainWnd,WM_VMB_DISCONNECT,0,0);
}


void sound_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{ soundExit();
  PostMessage(hMainWnd,WM_CLOSE,0,0);
}

struct inspector_def inspector[2] = {
    /* name size get_mem address num_regs regs */
	{"Registers",SOUND_MEM,sound_reg_read,sound_get_payload,sound_put_payload,hex_format,octa_chunk,-1,8,NUM_REGS,sound_regs},
	{0}
};


HANDLE haction;
CRITICAL_SECTION   action_section;


void init_device(device_info *vmb)
{	vmb_size = SOUND_MEM;
    haction =CreateEvent(NULL,FALSE,FALSE,NULL);
    InitializeCriticalSection (&action_section);
  vmb->poweron=sound_poweron;
  vmb->poweroff=sound_poweroff;
  vmb->disconnected=sound_disconnected;
  vmb->reset=sound_reset;
  vmb->terminate=sound_terminate;
  vmb->put_payload=sound_put_payload;
  vmb->get_payload=sound_get_payload;
  inspector[0].address=vmb_address;
}


 
void set_soundCtrl(void)
{ 
  vmb_debugi(VMB_DEBUG_INFO, "Setting soundCtrl 0x%X",SOUND_CTRL);
  switch (SOUND_CTRL&~SOUND_INTERRUPT)
  {  case SOUND_IGNORE: return;
     case SOUND_PLAYONCE:	PostThreadMessage(dwSoundThreadId, WM_SOUND_PLAYONCE,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUFFERS); return;
	 case SOUND_PLAYLOOP:	PostThreadMessage(dwSoundThreadId, WM_SOUND_PLAYLOOP,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUFFERS); return;	
	 case SOUND_PRELOAD:    
		 if (SOUND_BUF0) PostThreadMessage(dwSoundThreadId, WM_SOUND_PRELOAD,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUF0); 
		 if (SOUND_BUF1) PostThreadMessage(dwSoundThreadId, WM_SOUND_PRELOAD,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUF1); 
		 if (SOUND_BUF2) PostThreadMessage(dwSoundThreadId, WM_SOUND_PRELOAD,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUF2); 
		 if (SOUND_BUF3) PostThreadMessage(dwSoundThreadId, WM_SOUND_PRELOAD,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUF3); 
		 return;
	 case SOUND_UNLOAD:		
		 if (SOUND_BUF0) PostThreadMessage(dwSoundThreadId, WM_SOUND_UNLOAD,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUF0); 
		 if (SOUND_BUF1) PostThreadMessage(dwSoundThreadId, WM_SOUND_UNLOAD,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUF1); 
		 if (SOUND_BUF2) PostThreadMessage(dwSoundThreadId, WM_SOUND_UNLOAD,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUF2); 
		 if (SOUND_BUF3) PostThreadMessage(dwSoundThreadId, WM_SOUND_UNLOAD,SOUND_CTRL&SOUND_INTERRUPT,SOUND_BUF3); 
		 return;
	 case SOUND_CANCEL:		PostThreadMessage(dwSoundThreadId, WM_SOUND_CANCEL,0,0); return;
	 case SOUND_RESET:		PostThreadMessage(dwSoundThreadId, WM_SOUND_RESET,0,0); return;
  }
}



/* the thread serving dma requests is started when we 
   open the sound image and closed, when we close the sound Image */

static void soundWrite(void);
static void soundInterrupt(void);
static void buffer_load(int i);

/* from wimp3.c */
extern void mp3_buffer_done(void);
extern void start_mp3_sound(void);
extern void stop_mp3_sound(void);
static unsigned char buffers[4];
static int buffer_index=0; 
static int play_interrupts=0;
static enum {IDLE, MP3ONCE,PCM} mode;

#define is_loaded(i) (soundDma[i].loaded==soundDma[i].size)
#define needs_more_loading(i) (soundDma[i].loaded<soundDma[i].allocated)

static DWORD WINAPI sound_server(LPVOID dummy)
{  
   MSG msg;
   PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
   while (GetMessage(&msg,(HWND)-1,0,0))
   { switch (msg.message)
     { case WM_SOUND_IGNORE: break;
       case WM_SOUND_UNLOAD:
		 if (1<=msg.lParam && msg.lParam<=16)
		 { int i = msg.lParam-1;
		   if (soundDma[i].buffer) 
		   { free(soundDma[i].buffer);
		     soundDma[i].buffer=NULL;
		   }
		   soundDma[i].allocated=0;
		   soundDma[i].loaded=0;
		   soundDma[i].played=0;
		   if (msg.wParam)
			 soundInterrupt();
		 }
		 break;
	   case WM_SOUND_PRELOAD:
		 if (1<=msg.lParam && msg.lParam<=16 && !is_loaded(msg.lParam-1))
		 { buffer_load(msg.lParam-1);
		   if (needs_more_loading(msg.lParam-1))
			 PostThreadMessage(dwSoundThreadId, WM_SOUND_PRELOAD,msg.wParam,msg.lParam); 
		   else if (msg.wParam)
			 soundInterrupt();
		 }
         break;
       case WM_SOUND_CANCEL:
		 stop_mp3_sound();
		 break;
       case WM_SOUND_RESET:
		 stop_mp3_sound();
	     { int i;
           buffers[0]=buffers[1]=buffers[2]=buffers[3]=0;
           playing=finished=buffer_index=0;
           position=0;
	       mode = IDLE;
	       for (i=0;i<16;i++) 
	       { soundDma[i].address = 0;
		     soundDma[i].size = 0;
		     soundDma[i].loaded=0;
		     soundDma[i].played=0;
		     if (soundDma[i].buffer!=NULL) free(soundDma[i].buffer);
		     soundDma[i].buffer=NULL;
			 soundDma[i].allocated=0;
	       }
         }
	     break;
       case WM_SOUND_TERMINATE:	
       case WM_SOUND_PLAYONCE:
		   stop_mp3_sound(); /* just in case */
		   SET4(buffers, msg.lParam);
		   position=0;
		   play_interrupts=msg.wParam;
		   mode=MP3ONCE;
		   finished=playing;
		   buffer_index=0;
		   playing=buffers[buffer_index];
		   start_mp3_sound();
		   break;
       case WM_SOUND_PLAYLOOP:
	     break;
	   case MM_WOM_DONE: 
		 mp3_buffer_done();
		 break;
     } 
   }
   return 0;
}


void start_sound_server(void)
{  
    HANDLE hSoundThread;
    hSoundThread = CreateThread( 
            NULL,              // default security attributes
            0,                 // use default stack size  
            sound_server,        // thread function 
            NULL,             // argument to thread function 
            0,                 // use default creation flags 
            &dwSoundThreadId);   // returns the thread identifier 
        // Check the return value for success. 
    if (hSoundThread == NULL) 
      vmb_fatal_error(__LINE__, "Creation of sound thread failed");
/* in the moment, I really dont use the handle */
    CloseHandle(hSoundThread);
	do Sleep(1); while (!PostThreadMessage(dwSoundThreadId,WM_SOUND_RESET,0,0));
}


data_address da ={NULL,0,0,0,SECTOR_SIZE,STATUS_INVALID};


int mp3_input_read(int id, void *data, size_t size)
/* this function is called when we play an mp3 stream and need more data 
   0<=playing<=16 is the currently playing buffer, 0 if no buffer is playing
   position is the position in this buffer.
   buffers[0], ..., buffers[3], are the buffers to be played, eventualy as a loop
   buffer_index gives the current buffer.
*/
{  while (buffers[buffer_index]==0 || position>=soundDma[buffers[buffer_index]-1].size )
  { buffer_index++;
    position=0;
    finished=playing;
	if (buffer_index>3)
	{ buffer_index=0;
      if (mode==MP3ONCE ||
	      ((buffers[0]==0 || soundDma[buffers[0]-1].size==0)&& 
		  (buffers[1]==0 || soundDma[buffers[1]-1].size==0)&& 
		  (buffers[2]==0 || soundDma[buffers[2]-1].size==0)&& 
		  (buffers[3]==0 || soundDma[buffers[3]-1].size==0)))
	  { playing =0;
	    position=0;
		return 0;
	  }
	}
  }
  playing = buffers[buffer_index];
  while (needs_more_loading(playing-1)) buffer_load(playing-1);
  if (is_loaded(playing-1)) 
  { unsigned int available = (unsigned int)(soundDma[playing-1].allocated-position); 
	if (available<size) 
	{ size=available;
	  if (play_interrupts) soundInterrupt();
	}
	if (size>0) memmove(data, soundDma[playing-1].buffer+position, size);
    position=position+size;
    return size;
  }
  else
  { uint64_t address;
	unsigned int available = (unsigned int)(soundDma[playing-1].size-position); 
	if (available<size) 
	{ size=available;
	  if (play_interrupts) soundInterrupt();
	}
    /* now we have a buffer that is not preloaded */
    if (size>SECTOR_SIZE)
	  size = SECTOR_SIZE;
    address=soundDma[playing-1].address+position;
    da.data=data;
    da.address_lo = LOTETRA(address);
    da.address_hi = HITETRA(address);
    da.status = STATUS_VALID;
    da.size = size;
    vmb_load(&vmb,&da);
    vmb_wait_for_valid(&vmb,&da);
    if (da.status!=STATUS_VALID) 
    {  vmb_error(__LINE__,"cannot read sound data");
       return 0;
    }
    vmb_debugi(VMB_DEBUG_PROGRESS, "loaded %d byte of sound data",size);
    position=position+size;
    return size;
  }
}


/* Operating the Sound */




static void soundInit(void) 
/* called at power on and reset */
{
  vmb_debug(VMB_DEBUG_PROGRESS, "Initializing Sound");
  memset(mem,0,sizeof(mem));
  PostThreadMessage(dwSoundThreadId, WM_SOUND_RESET,0,0);
}



static void soundExit(void) 
/* called at power off, reset, disconnect, terminate */
{
  vmb_debug(VMB_DEBUG_PROGRESS, "Closing Sound");
  PostThreadMessage(dwSoundThreadId, WM_SOUND_CANCEL,0,0);
}

static void soundInterrupt(void)
{ 
    vmb_raise_interrupt(&vmb,interrupt_no);
    vmb_debug(VMB_DEBUG_PROGRESS, "Raised interrupt");
}



static void buffer_load(int i)
{ int size;
  uint64_t address;
  if (soundDma[i].loaded>= soundDma[i].size) return;
  if (soundDma[i].buffer==NULL) 
  { soundDma[i].buffer=(unsigned char *)malloc((size_t)soundDma[i].size);
    if (soundDma[i].buffer==NULL) return;
	soundDma[i].allocated=soundDma[i].size;
  }
  if (soundDma[i].allocated!=soundDma[i].size) 
  { unsigned char *buffer=(unsigned char *)realloc(soundDma[i].buffer,(size_t)soundDma[i].size);
    if (buffer==NULL)
    { free(soundDma[i].buffer);
      soundDma[i].buffer=NULL;
      soundDma[i].allocated=0;
      return;
    }
	else
	{ soundDma[i].buffer=buffer;
	  soundDma[i].allocated=soundDma[i].size;
	}
  }
  /* now we have a buffer with loaded < allocated == size */
  size= (int)(soundDma[i].allocated-soundDma[i].loaded);
  if (size>SECTOR_SIZE)
   size = SECTOR_SIZE;
     address=soundDma[i].address+soundDma[i].loaded;
  da.data=soundDma[i].buffer+soundDma[i].loaded;
  da.address_lo = LOTETRA(address);
  da.address_hi = HITETRA(address);
  da.status = STATUS_VALID;
  da.size = size;
  vmb_load(&vmb,&da);
  vmb_wait_for_valid(&vmb,&da);
  if (da.status!=STATUS_VALID) 
  { vmb_error(__LINE__,"cannot read sound buffer");
    return;
  }
  soundDma[i].loaded=soundDma[i].loaded+size;
  vmb_debugi(VMB_DEBUG_PROGRESS, "loaded %d byte of sound data",size);
}

#if 0

static void soundWrite(void) 
{ /* memory --> sound */
  if (soundPosition())
  { int i;
    uint64_t total=0; //soundCnt*SECTOR_SIZE;
    for (i=0; total>0; i++)
	{ uint64_t size = soundDma[i].size;
	  uint64_t address = soundDma[i].address;
	  while(size>0) 
	  { int part;
	    if (size>SECTOR_SIZE)
			part = SECTOR_SIZE;
		else
			part = (int)size;
		if (part>total)
		  size=part=(int)total;
		da.address_lo = LOTETRA(address);
        da.address_hi = HITETRA(address);
        da.status = STATUS_VALID;
        da.size = part;
        vmb_load(&vmb,&da);
        vmb_wait_for_valid(&vmb,&da);
        if (da.status!=STATUS_VALID) {
          vmb_error(__LINE__,"cannot read memory");
		  soundDone();
          return;
        }
//        if (fwrite(sector_buffer, 1, part, soundImage) != part)  {
//          vmb_error(__LINE__,"cannot write to sound");
//		  soundStatus=soundStatus| SOUND_ERR;
//		  soundDone();
//          return;
//        }
//        vmb_debugi(VMB_DEBUG_PROGRESS, "Wrote sector %d",(int)soundSct);
	    address = address+part;
	    size = size-part;
        total=total-part;
//	    { int d = (int)(soundCnt*SECTOR_SIZE-total)/SECTOR_SIZE;
//	      if (d>0)
//		  { soundCnt=soundCnt-d;
//            soundSct=soundSct+d;
//	        SET_SOUND_CNT(soundCnt);
//	        SET_SOUND_SCT(soundSct);
//	        mem_update(0x10,0x10);
//		  }
//	    }
	  }
	}
  }       
  soundDone();
}
#endif