/*
 * Copyright (C) 2001-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/LGPL-2.1
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "dcache.h"
#include "dcachei.h"

int
dcache_reload_vars(dcache *c)
{
	if (0==dcache_pos_fromdisk(c->mapped+DCACHE_OFF_NEWPTR,&c->n))
		goto not;
	if (0==dcache_pos_fromdisk(c->mapped+DCACHE_OFF_OLDPTR,&c->o))
		goto not;
	if (0==dcache_pos_fromdisk(c->mapped+DCACHE_OFF_LASTPTR,&c->l))
		goto not;
	if (0==dcache_uint32_fromdisk(c->mapped+DCACHE_OFF_INUSE,&c->inuse))
		goto not;
	if (0==dcache_uint32_fromdisk(c->mapped+DCACHE_OFF_INSERTED,&c->inserted))
		goto not;
	if (0==dcache_uint32_fromdisk(c->mapped+DCACHE_OFF_DELETED,&c->deleted))
		goto not;
	if (0==dcache_uint32_fromdisk(c->mapped+DCACHE_OFF_MOVES,&c->moves))
		goto not;
	return 0;
  not:
	errno=EINVAL;
	return -1;
}

int
dcache_store_vars(dcache *c)
{
	dcache_pos_todisk(c->mapped+DCACHE_OFF_NEWPTR,c->n);
	dcache_pos_todisk(c->mapped+DCACHE_OFF_OLDPTR,c->o);
	dcache_pos_todisk(c->mapped+DCACHE_OFF_LASTPTR,c->l);
	dcache_uint32_todisk(c->mapped+DCACHE_OFF_INUSE,c->inuse);
	dcache_uint32_todisk(c->mapped+DCACHE_OFF_INSERTED,c->inserted);
	dcache_uint32_todisk(c->mapped+DCACHE_OFF_DELETED,c->deleted);
	dcache_uint32_todisk(c->mapped+DCACHE_OFF_MOVES,c->moves);
	return 0;
}


int
dcache_init(dcache *c, int fd, int forwrite)
{
	struct stat st;
	char *m;
	char buf[DCACHE_MASTERTABLE];
	int l;
	dcache_uint32 ui32;
	dcache_uint32 elements;
	dcache_pos    maxsize;
	dcache_uint32 mapsize;
	if (-1==fstat(fd,&st)) 
		return -1;
	if (-1==dcache_seek(fd,0))
		return -1;
	if (st.st_size<DCACHE_MASTERTABLE) 
		goto not;
	l=dcache_read(fd,buf,sizeof(buf));
	if (l!=sizeof(buf))
		return -1;
	if (0==dcache_uint32_fromdisk(buf+DCACHE_OFF_MAGIC0,&ui32)) goto not;
	if (ui32!=DCACHE_MAGIC0) goto not;
	if (0==dcache_uint32_fromdisk(buf+DCACHE_OFF_MAGIC1,&ui32)) goto not;
	if (ui32!=DCACHE_MAGIC1) goto not;
	if (0==dcache_uint32_fromdisk(buf+DCACHE_OFF_VERSION,&ui32)) goto not;
	if (ui32!=DCACHE_VERSION) goto not;
	if (0==dcache_uint32_fromdisk(buf+DCACHE_OFF_ELEMENTS,&elements)) goto not;
	if (0==dcache_pos_fromdisk(buf+DCACHE_OFF_MAXSIZE,&maxsize)) goto not;

#define E_SIZE (sizeof(dcache_pos) +sizeof(dcache_uint32))
	if (((0xffffffff-DCACHE_MASTERTABLE)/elements) < E_SIZE)
		goto not; /* overflow */

	mapsize=DCACHE_MASTERTABLE 
	        +elements * (sizeof(dcache_pos) +sizeof(dcache_uint32));

	m=mmap(0,mapsize,
		forwrite ? PROT_READ|PROT_WRITE : PROT_READ ,
		MAP_SHARED,fd,0);
	if (m==(char *)-1)
		return -1;

	c->fd=fd;
	c->loop=0;
	c->size=st.st_size;
	c->mapsize=mapsize;
	c->mapped=m;
	c->elements=elements;
	c->maxsize=maxsize;
	c->flag_autosync=1;
	c->t_buf=0;

	if (0==dcache_pos_fromdisk(buf+DCACHE_OFF_DATAOFF,&c->dataoffset))
		goto not_unmap;
	if (0==dcache_uint32_fromdisk(buf+DCACHE_OFF_SLOTUSE,&c->slotuselimit))
		goto not_unmap;
	if (-1==dcache_reload_vars(c))
		goto not_unmap;

	c->ep=(dcache_pos *)    (m+DCACHE_MASTERTABLE);
	c->hp=(dcache_uint32 *) (c->ep+c->elements);
	if (c->dataoffset+c->maxsize > st.st_size)
		goto not_unmap;
	
	return 0;
  not_unmap:
	munmap(m,mapsize);
  not:
	errno=EINVAL;
	return -1;
}
