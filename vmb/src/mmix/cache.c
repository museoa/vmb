/*
    Copyright 2005 Martin Ruckert
    
    ruckertm@acm.org

    This file is part of the MMIX Motherboard project

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "bus-arith.h"
#include "bus-util.h"
#include "address.h"
#include "cache.h"


/* loading data from physical memory */

static void load_uncached_memory(unsigned char *data, int size, octa address)
     /* load size byte from the given physical address
        into the memory pointed to by data */

{ unsigned char a[8];
  inttochar(address.h,a);
  inttochar(address.l,a+4);
  load_bus_data(bus_fd,a,data,size);
}

static void store_uncached_memory(unsigned char *data, int size, octa address)
     /* store size byte to the given physical address
        from the memory pointed to by data */

{ unsigned char a[8];
  inttochar(address.h,a);
  inttochar(address.l,a+4);
  store_bus_data(bus_fd,a,data,size);
}

static void char_to_octa(int size, unsigned char *d,octa *data, int signextension)
{  if (size==8)
  { data->h = chartoint(d);
    data->l = chartoint(d+4);
  }
  else
    { if (signextension && (d[0]&0x80))
      { signed int i;
        data->h = 0xFFFFFFFF;
        i = chartoint(d);
        if (size < 4) i= i>>((4-size)*8);
	data->l = i;
      }
      else
      { unsigned int i;
        data->h = 0;
        i = chartoint(d);
        if (size < 4) i= i>>((4-size)*8);
	data->l = i;
      }
   }
}


void load_uncached_data(int size, octa *data, octa address, int signextension)
/* load size (1,2,4,or 8) byte into data from the given physical address 
*/
{ unsigned char d[8];
  address.l=address.l&~(size-1);  /* round down to next alignment */
  load_uncached_memory(d,size,address);
  char_to_octa(size,d,data,signextension);
}


void store_uncached_data(int size, octa data, octa address)
/* store an octa, tetra, wyde, or byte (depending on size)
   in data to the given physical address 
   raise an interrupt if there is a problem
*/
{ unsigned char d[8]= {0};
  address.l=address.l&~(size-1);  /* round down to next alignment */
  if (size == 8)
  { inttochar(data.h,d);
    inttochar(data.l,d+4);
    store_uncached_memory(d, size, address);
  }
  else
  { inttochar(data.l,d);
    store_uncached_memory(d+(4-size), size, address);
  }
}

/* the cache */

#define WAYS     2       /* a n-way cache */
#define LINEBITS 6       /* bits to index a cache line */
#define LINESIZE (1<<LINEBITS) /* bytes per cache line */
#define LINEMASK (LINESIZE-1) /* mask to get line bits */
#define CACHEBITS 13     /* bits to index the cache */
#define CACHEMASK ((1<<CACHEBITS)-1)  /* mask to get cache bits */
#define CACHEOFFSET(address) (address.l & LINEMASK)
#define CACHEINDEX(address)  ((address.l & CACHEMASK)>>LINEBITS)
#define CACHESIZE (1<<(CACHEBITS-LINEBITS))

typedef struct {
  unsigned char data[LINESIZE]; 
  octa address;  
  unsigned int use;
  unsigned int dirty;
} cache_line;

typedef struct {
  unsigned int access;
  cache_line line[WAYS];
} cache[CACHESIZE];

static cache instruction_cache={{0}};
static cache data_cache={{0}};


void clear_all_data_cache(void)
{ int i,k;
 for (i=0;i<CACHESIZE;i++)
 { data_cache[i].access=0;
   for (k=0;k<WAYS;k++)
   { data_cache[i].line[k].use=0;
     data_cache[i].line[k].dirty=0;      
   }
 }
}

void clear_all_instruction_cache(void)
{ int i,k;
 for (i=0;i<CACHESIZE;i++)
 { instruction_cache[i].access=0;
   for (k=0;k<WAYS;k++)
      instruction_cache[i].line[k].use=0;
 }
}



static cache_line *cache_lookup(cache c, octa address)
     /* return a matching cache line or NULL */
{ 
  int i = CACHEINDEX(address);
  cache_line *line = c[i].line;
  int w;
  for (w=0;w<WAYS;w++)
    if (line[w].use != 0 &&
        line[w].address.h == address.h &&
        ((line[w].address.l&~CACHEMASK) == (address.l&~CACHEMASK)))
      { if (++c[i].access==0) ++c[i].access;
        line[w].use = c[i].access;
        return line+w;
      }
  return NULL;
}

static cache_line *cache_lru(cache c, octa address)
     /* return the least recently used cache line */
{
  int i = CACHEINDEX(address);
  cache_line *line = c[i].line;
  int access = c[i].access;
  int lru = 0;
  int age = access-line[lru].use;
  int w;
  for (w=1;w<WAYS;w++)
    if (access-line[w].use > age)
      { age = access-line[w].use; lru=w; }
  if (++access==0) ++access;
  line[lru].use = c[i].access=access;
  return line+lru;
}

static void cache_load(cache_line *line, octa address)
{ line->address.h = address.h;
  line->address.l = address.l & ~LINEMASK;
  load_uncached_memory(line->data,LINESIZE,line->address);
  line->dirty=0;
}

static void cache_store(cache_line *line)
{ store_uncached_memory(line->data,LINESIZE,line->address);
  line->dirty=0;
}

void write_all_data_cache(void)
{ int i,k;
 for (i=0;i<CACHESIZE;i++)
 { for (k=0;k<WAYS;k++)
   if(data_cache[i].line[k].dirty)
     cache_store(&(data_cache[i].line[k]));
 }
}

void load_cached_instruction(tetra *instruction, octa address)
{ cache_line *line;
  line = cache_lookup(instruction_cache,address);
  if (line==NULL)
  { line = cache_lru(instruction_cache,address);
    cache_load(line,address);
  }
  *instruction = chartoint(line->data + (CACHEOFFSET(address)&~3));
}

void load_cached_data(int size, octa *data, octa address, int signextension)
{ cache_line *line;
  line = cache_lookup(data_cache,address);
  if (line==NULL)
  { line = cache_lru(data_cache,address);
    if (line->dirty)
      cache_store(line);
    cache_load(line,address);
  }
  char_to_octa(size,line->data + (CACHEOFFSET(address)&~(size-1)),data,signextension);
}


void store_cached_data(int size, octa data, octa address)
{ cache_line *line;
  line = cache_lookup(data_cache,address);
  if (line==NULL)
  { line = cache_lru(data_cache,address);
    if (line->dirty)
      cache_store(line);
    cache_load(line,address);
  }
  address.l=address.l&~(size-1);  /* round down to next alignment */
  if (size == 8)
  { inttochar(data.h,line->data + (CACHEOFFSET(address)&~7));
    inttochar(data.l,line->data + (CACHEOFFSET(address)&~7)+4);
  }
  else
  { unsigned char d[4];
    inttochar(data.l,d);
    memcpy(line->data + (CACHEOFFSET(address)&~(size-1)),d+(4-size),size);
  }
  line->dirty=1;
}

void write_data_cache(octa address, int size)
{ cache_line *line;
 int i;
 unsigned int u;
 for (i = -(address.l&LINEMASK); i<size;i=i+LINESIZE)
 { line = cache_lookup(data_cache,address);
   if (line!=NULL && line->dirty)
      cache_store(line);
   u= address.l + LINESIZE;
   if (u < address.l)
     address.h++;
   address.l=u;
 }
}

void clear_data_cache(octa address, int size)
{ cache_line *line;
 int i;
 unsigned int u;
 for (i = -(address.l&LINEMASK); i<size;i=i+LINESIZE)
 { line = cache_lookup(data_cache,address);
   if (line!=NULL)
     line->use=0;
   u= address.l + LINESIZE;
   if (u < address.l)
     address.h++;
   address.l=u;
 }
}

void clear_instruction_cache(octa address, int size)
{ cache_line *line;
 int i;
 unsigned int u;
 for (i = -(address.l&LINEMASK); i<size;i=i+LINESIZE)
 { line = cache_lookup(instruction_cache,address);
   if (line!=NULL)
     line->use=0;
   u= address.l + LINESIZE;
   if (u < address.l)
     address.h++;
   address.l=u;
 }
}

void read_instruction_cache(octa address, int size)
{ cache_line *line;
  int i;
  unsigned int u;
  for (i = -(address.l&LINEMASK); i<size;i=i+LINESIZE)
  { line = cache_lookup(instruction_cache,address);
    if (line==NULL)
    { line = cache_lru(instruction_cache,address);
      cache_load(line,address);
    }
    u= address.l + LINESIZE;
    if (u < address.l)
      address.h++;
    address.l=u;
  }
}


/* we need to define some hooks */

int process_poweron(void)
{ return 0; }

int process_poweroff(void)
{ return 0; }

int process_reset(void)
{ return 0; }


int process_interrupt(unsigned char interrupt)
{ return 0; }

int reply_payload(unsigned char address[8],int size, unsigned char *payload)
{ return 1;
}


int write_request(unsigned char a[8], int s, unsigned char p[MAXPAYLOAD])
{ return 0; }


int read_request( unsigned char a[8], int s, unsigned char slot, unsigned char p[MAXPAYLOAD])
{ return 0; }
