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
#include "message.h"
#include "mmix-internals.h"
#include "address.h"
#include "mmix-bus.h"

/* CONSTANTS */
#define EXEC_BIT 0x1
#define WRITE_BIT 0x2
#define READ_BIT 0x4
#define PAGE_FAULT_BIT 0x8


/* TYPES */

/* Page Table Entry */
typedef struct {
  /* int x;  not implemented */
  octa base;    /* base address with x, y, n, p fields set to zero */
  /* int y;  not implemented */
  unsigned int n;
  int p;
} PTE;



/* Translation Cache Entry */
#define TCSIZE 64

typedef struct {
  int i;
  int n;
  octa base;
  octa phys;
  int p;
} TCE;


/* Tranclation Cache */
typedef struct { int last; TCE tce[TCSIZE];} TC;


/* GLOBAL VARIABLES */

/* the two translation caches for instructions and data */
static TC exec_tc={0,{{0}}}, data_tc={0,{{0}}}; 

void clear_all_data_vtc(void)
{ int i;
  for (i=0;i<TCSIZE;i++)
    data_tc.tce[i].p=0; 
}


void clear_all_instruction_vtc(void)
{ int i;
  for (i=0;i<TCSIZE;i++)
    exec_tc.tce[i].p=0; 
}



/* AUXILIAR FUNCTIONS */


static void unpack_pte(PTE *e,octa *a, int s)
     /* unpack a PTE from an octa a */
{ e->p = a->l& 0x07;
  e->n = (a->l>>3)& 0x3FF;
  if (s<32)
    { unsigned int mask = ~((1<<s)-1); 
      e->base.l = a->l & mask;
      e->base.h = a->h & 0xFFFF;
    }
  else if (s==32)
    { e->base.l = 0;
      e->base.h = a->h & 0xFFFF;
    }
  else
    { unsigned int mask = ~((1<<(s-32))-1); 
      e->base.l = 0;
      e->base.h = a->h & 0xFFFF & mask;
    }
}




/* Handling a page table entry */

static int translate_v2p(int b[5], int s, octa *r, unsigned int n, int i, octa *address)
/* given the components b[i], s,r,n, and i from rV,
   translate the virtual address into 
   a pysical address and return the protection code p.
   return 0 if there is no translation.
   use the alorithm as given by Don Knuth in section 47 of mmix-doc.
   modified according to my letter to don.
     */
{  octa base, pte, ptp;
   int limit;
   int a[5];
   PTE e;

   base.h = r->h;
   base.l = r->l + (b[i]<<13); /* address of first page table */
   limit = (b[i+1]-b[i])*2;

   address->h = address->h&0x1FFFFFFF; /*remove sign and segment */ 
   *address = shift_right(*address,s,1); /* remove offset get a4a3a2a1a0 */

   if (address->h==0 && address->l==0)
     limit=limit+1;

   if (limit <= 0) goto pagetable_error;
   limit = limit-2;

   ptp = base;
   a[0] = address->l&0x3FF;
   *address = shift_right(*address,10,1); /* remove a0 */   
   if (address->h!=0 || address->l!=0)
     { if (limit <= 0) goto pagetable_error;
       limit = limit-2;
       base.l = base.l+0x2000;
       ptp = base;
       a[1] =  address->l&0x3FF;
       *address = shift_right(*address,10,1); /* remove a1 */
       if (address->h!=0 || address->l!=0)
	 { if (limit <= 0) goto pagetable_error;
           limit = limit-2;
           base.l = base.l+0x2000;
           ptp = base;
           a[2] =  address->l&0x3FF;
           *address = shift_right(*address,10,1); /* remove a2 */
           if (address->h!=0 || address->l!=0)
	     { if (limit <= 0) goto pagetable_error;
               limit = limit-2;
               base.l = base.l+0x2000;
               ptp = base;
               a[3] =  address->l&0x3FF;
               *address = shift_right(*address,10,1); /* remove a3 */
               a[4] =  address->l&0x3FF;
	       if (a[4] != 0)
		 { if (limit <= 0) goto pagetable_error;
                   limit = limit-2;
                   base.l = base.l+0x2000;
                   ptp = base;
	           base.l = base.l+8*a[4];
                   load_uncached_data(8,&base,base,0);
                   if (((base.h&0x80000000)==0) || ((base.l>>3)&0x3FF)!=n)
		     goto pagetable_error;
                   base.l = base.l & (~0x1FFF);
                   base.h = base.h & 0x7FFFFFFF;
		 }
	         base.l = base.l+8*a[3];
                 load_uncached_data(8,&base,base,0);
                 if (((base.h&0x80000000)==0) || ((base.l>>3)&0x3FF)!=n)
		   goto pagetable_error;
                 base.l = base.l & (~0x1FFF);
                 base.h = base.h & 0x7FFFFFFF;
	     }
             base.l = base.l+8*a[2];
             load_uncached_data(8,&base,base,0);
             if (((base.h&0x80000000)==0) || ((base.l>>3)&0x3FF)!=n)
	       goto pagetable_error;
             base.l = base.l & (~0x1FFF);
             base.h = base.h & 0x7FFFFFFF;
	 }
         base.l = base.l+8*a[1];
         load_uncached_data(8,&base,base,0);
         if (((base.h&0x80000000)==0) || ((base.l>>3)&0x3FF)!=n)
           goto pagetable_error;
         base.l = base.l & (~0x1FFF);
         base.h = base.h & 0x7FFFFFFF;
     }
   base.l = base.l+8*a[0];
   load_uncached_data(8,&pte,base,0);
   if (pte.h==0 && pte.l==0)
     return 0;
   unpack_pte(&e,&pte,s);
   if (e.n != n) /* this is an error in the page table structure */
     goto pagetable_error;
   *address = e.base;
   return e.p;
 
pagetable_error:
   g[rQ].l = g[rQ].l | BIT(INT_PAGEERROR);
   return 0;
}

/* Handling the Translation Cache */

TCE *TCE_lookup(TC *tc, int i, int n, octa *base)
/* returns NULL if there is no corresponding pte in tc */
{ int k;
  TCE *tce;
  for (k=0;k<TCSIZE;k++)
  { tce = tc->tce + ((tc->last +k) & (TCSIZE-1));
    if ( tce->p != 0 &&
         tce->i == i && 
         tce->n == n &&
         tce->base.h == base->h &&
         tce->base.l == base->l )
      return tce;
  }    
  return NULL;
} 

void TC_store(TC *tc, int i, int n, octa *base, octa *phys, int p)
{ tc->last = (tc->last +1) & (TCSIZE-1);
  tc->tce[tc->last].p = p;
  tc->tce[tc->last].i = i;
  tc->tce[tc->last].n = n;
  tc->tce[tc->last].base = *base;
  tc->tce[tc->last].phys = *phys;
}


static int TC_update(TC *tc, int i, int n, octa *base, int p)
/* updates the access code p in the translation cache.
   if p == 0 it will delete a corresponding entry
*/
{ TCE *tce = TCE_lookup(tc,i,n,base);
  if (tce==NULL) return 0;
  tce->p = p;
  return 1;
} 


int TC_translate(TC *tc, int s, int i, int n, octa *base)
     /* return 0 if not found;
        return protection on success and replace the virtual base by the pysical
     */

{ TCE *tce;
  tce = TCE_lookup(tc,i,n,base);
  if (tce==NULL)
  { int b[5];
    octa r,phys;
    int p;
    b[0] = 0;
    b[1] = (g[rV].h>>28)&0xF;
    b[2] = (g[rV].h>>24)&0xF;
    b[3] = (g[rV].h>>20)&0xF;
    b[4] = (g[rV].h>>16)&0xF;
    r.h  = g[rV].h&0xFF;
    r.l  = g[rV].l&~0x1FFF;
    phys=*base;
    p=translate_v2p(b,s,&r,n,i,&phys);
    if (p==0)
      return PAGE_FAULT_BIT;
    TC_store(tc,i, n, base, &phys, p);
    *base = phys;
    return p;
  }
  *base = tce->phys;
  return tce->p;
}



#if 0

octa update_vtc(octa key)
/* implements the LDVTS instruction accordint to mmix-doc */
{  int i;
   int n;
   int p;
   octa x;
   i = (key.h>>29) & 0x03;
   n = (key.l>>3) & 0x3FF;
   p = key.l & 0x07;
   key.h = key.h &0x1FFFFFFF;
   key.l = key.l & ~0x01FF;
   x.h=0;
   x.l = TC_update(&data_tc,i,n, &key,p)<< 1;
   x.l = x.l | TC_update(&exec_tc,i,n, &key,p);
   return x;
}

#else
octa update_vtc(octa key)
/* implements the LDVTS instruction
   according to LDTV.txt proposal 2
 */
{  int i;
   int n;
   int p;
   int s; 
   octa x;
   s    = (g[rV].h>>8)&0xFF;  /* extract the page size from rV */
   i = (key.h>>29) & 0x03;
   n = (key.l>>3) & 0x3FF;
   p = key.l & 0x07;
   key.h = key.h &0x1FFFFFFF;
   key.l = key.l & ~0x01FF;
   x.h=x.l=-1;

   if (p==0) /*delete from both caches*/
     { TC_update(&data_tc,i,n, &key,0);
       TC_update(&exec_tc,i,n, &key,0);
     }
   else if (p == 1) /* check data VT cache, provide translation if present */
     { TCE *tce;
       tce = TCE_lookup(&data_tc,i,n,&key);
        if (tce!=NULL)
	{  x.h = tce->phys.h;
	   x.l = tce->phys.l | tce->p; 
        }
     }
   else if (p == 2) /* check instruction VT cache, provide translation if present */
     { TCE *tce;
       tce = TCE_lookup(&exec_tc,i,n,&key);
        if (tce!=NULL)
	{  x.h = tce->phys.h;
	   x.l = tce->phys.l | tce->p; 
        }
     }
   else if (p == 3) 
      /* lazy update both caches.
         if a matching entry is present in one of the caches, reread the translation
         from memory and replace the matching enties in both caches. */
     {
     }
  else if (p == 4) 
      /* update of data VT cache. reread translation from memory and place
         into data VT cache. replace an existing instruction VT cache entry. */
     {
     }
  else if (p == 5) 
      /* update of instruction VT cache. reread translation from memory and place
         into instruction VT cache. replace an existing data VT cache entry. */
     {
     }
  else if (p == 6) 
      /* read data translation, read a translation from the data cache, if not present
         read the translation from memory and keep the result in the data VT cache */
     { p= TC_translate(&data_tc, s, i, n, &key);
       if (!(p&PAGE_FAULT_BIT))
       { x.h = key.h;
         x.l = key.l | p;
       }
     }
  else if (p == 7) 
      /* read instruction translation, read a translation from the instruction cache,
         if not present, read the translation from memory 
         and keep the result in the instruction VT cache */
     { p= TC_translate(&exec_tc, s, i, n, &key);
       if (!(p&PAGE_FAULT_BIT))
       { x.h = key.h;
         x.l = key.l | p;
       }
     }
    return x;
}


#endif



static void unpack_address(octa *virt, int s, int *i, octa *base, octa *offset)
     /* unpack a virtual address into segment i, base and offset */
{ *i = (unsigned)virt->h >> 29;
  if (s > 32 )
  { unsigned int mask;
     mask = (1 << (s-32))-1;
     base->h = virt->h & ~mask & 0x1FFFFFFF;
     base->l = 0;
     offset->h = virt->h & mask;
     offset->l = virt->l;
  }
  else if (s == 32)
  { base->h = virt->h & 0x1FFFFFFF;
     base->l = 0;
     offset->h = 0;
     offset->l = virt->l;
  }
  else
  { unsigned int mask;
     mask = (1 << s)-1;
     base->h = virt->h & 0x1FFFFFFF;
     base->l = virt->l & ~mask;
     offset->h = 0;
     offset->l = virt->l & mask;
  }
}


static void store_translation(TC *tc,octa *virt, octa *phys)
{ int vi, pi;
 int n; 
 int s;
 octa vbase, pbase;
 octa voffset, poffset;
 n    = (g[rV].l>>3)&0x3FF;
 s    = (g[rV].h>>8)&0xFF;  /* extract the page size from rV */

 unpack_address(virt, s, &vi, &vbase, &voffset);
 unpack_address(phys, s, &pi, &pbase, &poffset);
 TC_store(tc, vi, n, &vbase, &pbase,poffset.l&0x7);
}

void store_data_translation(octa *virt, octa *phys)
{
  store_translation(&data_tc,virt, phys);
}

void store_exec_translation(octa *virt, octa *phys)
{
  store_translation(&exec_tc,virt, phys);
}


int translate_address(octa *address, TC *tc)
     /* the virtual address is repaced be the physical address.
	return the protection p on success, 0 on failure.
     */
{ /* Use octa g[rV] for the special rV register */
  int s,i,p;
  octa base, offset;
  int n;
  n    = (g[rV].l>>3)&0x3FF;
  s    = (g[rV].h>>8)&0xFF;  /* extract the page size from rV */
  unpack_address(address, s, &i, &base, &offset);
  p= TC_translate(tc, s, i, n, &base);
  *address = oplus(base,offset);
  return p;
}

int load_instruction(tetra *instruction, octa address)
/* load a tetra into data from the given virtual address 
   raise an interrupt if there is a problem and load 0.
*/
{ if (address.h&0x80000000)
    /* negative addresses are maped to physical adresses by suppressing the sign bit */
    address.h= address.h&0x7FFFFFFF;
  else
  { int p = translate_address(&address, &exec_tc);
    if (p&PAGE_FAULT_BIT)
    { *instruction = 0;
      g[rQ].l = g[rQ].l | BIT( INT_PAGEFAULT);
      return 0;
    }
    else if (!(p&EXEC_BIT)) 
    { *instruction = 0;
      g[rQ].h = g[rQ].h | BIT(INT_EXEC);
      return 0;
    }
  }
  if (address.h & 0xFFFF0000)
  { /* load uncached */
    octa data;
    load_uncached_data(4,&data,address,0);
    *instruction = data.l;
  }
  else
    load_cached_instruction(instruction,address);
  return 1;
}

int load_data(int size, octa *data, octa address,int signextension)
/* size may be 1, 2, 4 or 8 
   load a byte, wyde, tetra, or octa into data from the given virtual address 
   raise an interrupt if there is a problem
*/

{ if (address.h&0x80000000)
    /* negative addresses are maped to physical adresses by suppressing the sign bit */
    address.h= address.h&0x7FFFFFFF;
  else 
  { int p = translate_address(&address, &data_tc);
    if (p&PAGE_FAULT_BIT)
    { data->h=data->l = 0;
      g[rQ].l = g[rQ].l | BIT( INT_PAGEFAULT);
      return 0;
    }
    else if (!(p&READ_BIT)) 
    { data->h=data->l = 0;
      g[rQ].h = g[rQ].h | BIT(INT_READ);
      return 0;
    }
  }
  if (address.h & 0xFFFF0000)
    load_uncached_data(size,data,address,signextension);
  else
    load_cached_data(size,data,address,signextension);
  return 1;
}


int store_data(int size,octa data, octa address)
/* store an octa from data into the given virtual address 
   raise an interrupt if there is a problem
*/
{ if (address.h&0x80000000)
    /* negative addresses are maped to physical adresses by suppressing the sign bit */
    address.h= address.h&0x7FFFFFFF;
  else
  { int p = translate_address(&address, &data_tc);
    if (p&PAGE_FAULT_BIT)
    { g[rQ].l = g[rQ].l | BIT( INT_PAGEFAULT);
      return 0;
    }
    else if (!(p&WRITE_BIT)) 
    { g[rQ].h = g[rQ].h | BIT(INT_WRITE);
      return 0;
    }
  }
  if (address.h & 0xFFFF0000)
    store_uncached_data(size,data,address);
  else
    store_cached_data(size,data,address);
  return 1;
}


void write_data(octa address,int size)
/* make sure size bytes starting at address are writen
   from the data cache to memory 
   no exceptions are raised if writing is not possible
*/
{ if (address.h&0x80000000)
    /* negative addresses are maped to physical adresses by suppressing the sign bit */
    address.h= address.h&0x7FFFFFFF;
  else
  { int p = translate_address(&address, &data_tc);
    if (p&PAGE_FAULT_BIT ||p==0) 
      return;
  }
  if (address.h & 0xFFFF0000)
    return; /* these addresses are not cached */
  else
    write_data_cache(address, size);
}

void delete_data(octa address,int size)
     /* make sure size bytes starting at address are deleted
        from the data cache */
{ if (address.h&0x80000000)
    /* negative addresses are maped to physical adresses by suppressing the sign bit */
    address.h= address.h&0x7FFFFFFF;
  else
  { int p = translate_address(&address, &data_tc);
    if (p&PAGE_FAULT_BIT ||p==0) 
      return;
  }
  if (address.h & 0xFFFF0000)
    return; /* these addresses are not cached */
  else
    clear_data_cache(address, size);
}

void delete_instruction(octa address,int size)
     /* make sure size bytes starting at address are deleted
        from the instruction cache */
{ if (address.h&0x80000000)
    /* negative addresses are maped to physical adresses by suppressing the sign bit */
    address.h= address.h&0x7FFFFFFF;
  else
  { int p = translate_address(&address, &data_tc);
    if (p&PAGE_FAULT_BIT ||p==0) 
      return;
  }
  if (address.h & 0xFFFF0000)
    return; /* these addresses are not cached */
  else
    clear_instruction_cache(address, size);
}

void prego_instruction(octa address,int size)
     /* make sure size bytes starting at address are read into
        the instruction cache */
{ if (address.h&0x80000000)
    /* negative addresses are maped to physical adresses by suppressing the sign bit */
    address.h= address.h&0x7FFFFFFF;
  else
  { int p = translate_address(&address, &data_tc);
    if (p&PAGE_FAULT_BIT || !(p&EXEC_BIT)) 
      return;
  }
  if (address.h & 0xFFFF0000)
    return; /* these addresses are not cached */
  else
    prego_instruction_cache(address, size);
}
