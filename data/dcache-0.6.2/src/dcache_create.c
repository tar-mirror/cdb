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
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include "dcache.h"
#include "dcachei.h"
#include "auto-have_ftruncate.h"

#define BUFSIZE 16384

/* number of slots allowed to be used. The remainding ones are left free. */
#define SLOTUSELIMIT(x) ((x)/4*3)

/* write buf multiple times until bytes bytes have been written */
static int 
writeloop(int fd, const char *buf, dcache_reclen bufsize, 	
	dcache_pos bytes)
{
	while (bytes) {
		dcache_reclen written;
		written=dcache_write(fd, buf, bytes > bufsize ? bufsize : bytes);
		if (written==-1)
			return -1;
		bytes-=written;
	}
	return 0;
}

int
dcache_create_fd(int fd, dcache_pos maxsize, 
	dcache_uint32 elements, int do_hole)
{
	unsigned int i;
	char buf[BUFSIZE];
	dcache_pos off;
	dcache_pos lslots;
	dcache_pos dataoffset;
	if (0==SLOTUSELIMIT(elements) || DC_HEADLEN+1 > maxsize) { 
		errno=EINVAL;
		return -1;
	}

	if (-1==dcache_seek(fd, 0))
		return -1;
#ifdef HAVE_FTRUNCATE
	if (-1==ftruncate(fd, 0))
		return -1;
#endif

	lslots=sizeof(dcache_pos)+sizeof(dcache_uint32);
	lslots*=elements;

	/* can't use getpagesize: our page size may be different from that
	 * on other systems. 
	 */
	dataoffset=((DCACHE_MASTERTABLE+lslots+DCACHE_MAX_PAGESIZE-1)
		 / DCACHE_MAX_PAGESIZE )*DCACHE_MAX_PAGESIZE;

	/* it's not really dataoffset i worry about, but i use uint32 
	 * to get to the right hash and pointer slots. */
	if (DCACHE_UINT32_MAX < dataoffset) {
		errno=EINVAL;
		return -1;
	}

	for (i=0;i<sizeof(buf);i++) 
		buf[i]=0;

	/* prepare header area */
	off= dcache_uint32_todisk(buf+DCACHE_OFF_MAGIC0,DCACHE_MAGIC0);
	off+=dcache_uint32_todisk(buf+DCACHE_OFF_MAGIC1,DCACHE_MAGIC1);
	off+=dcache_uint32_todisk(buf+DCACHE_OFF_VERSION,DCACHE_VERSION);
	off+=dcache_uint32_todisk(buf+DCACHE_OFF_ELEMENTS,elements);
	off+=dcache_uint32_todisk(buf+DCACHE_OFF_SLOTUSE,SLOTUSELIMIT(elements));
	off+=dcache_pos_todisk   (buf+DCACHE_OFF_MAXSIZE,maxsize);
	off+=dcache_pos_todisk   (buf+DCACHE_OFF_NEWPTR,0); /* n */
	off+=dcache_pos_todisk   (buf+DCACHE_OFF_OLDPTR,maxsize); /* o */
	off+=dcache_pos_todisk   (buf+DCACHE_OFF_LASTPTR,0); /* l */
	off+=dcache_pos_todisk   (buf+DCACHE_OFF_DATAOFF,dataoffset); 
	off+=dcache_uint32_todisk(buf+DCACHE_OFF_INUSE,0); /* inuse */
	off+=dcache_uint32_todisk(buf+DCACHE_OFF_INSERTED,0); /* inserted */
	off+=dcache_uint32_todisk(buf+DCACHE_OFF_DELETED,0); /* deleted */
	off+=dcache_uint32_todisk(buf+DCACHE_OFF_MOVES,0); /* moves */

	if (-1==writeloop(fd,buf,sizeof(buf),DCACHE_MASTERTABLE))
		return -1;

	/* write entry pointer and hash areas */
	for (i=0;i<sizeof(buf);i++)
		buf[i]=0; /* avoid memset */

	if (-1==writeloop(fd,buf,sizeof(buf),lslots))
		return -1;

	/* write data space */
	if (do_hole) {
		if (-1==dcache_seek(fd,dataoffset+maxsize)) 
			return -1;
		if (-1==dcache_write(fd,buf,1))
			return -1;
		return fd;
	}
	/* the next one could create a hole in a space we never use anyway. */
	if (-1==dcache_seek(fd,dataoffset)) 
		return -1;
	if (-1==writeloop(fd,buf,sizeof(buf),maxsize))
		return -1;

#ifndef HAVE_FTRUNCATE
	/* write \0 so that there is no valid transaction at the end of
	 * the dataspace */
	if (-1==dcache_write(fd,buf,1))
		return -1;
#endif
	return fd;
}

int
dcache_create_name(const char *fname, int mode, dcache_pos maxsize, 
	dcache_uint32 elements, int do_hole)
{
	int fd;

	fd=open(fname,O_RDWR|O_CREAT|O_TRUNC,mode);
	if (-1==fd) 
		return -1;
	if (-1==dcache_create_fd(fd,maxsize,elements,do_hole)) {
		int e=errno;
		unlink(fname);
		close(fd);
		errno=e;
		return -1;
	}
	return fd;
}

