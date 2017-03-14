/*
 * Copyright (C) 2001-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/LGPL-2.1
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include "dcache.h"
#include "dcachei.h"

/* note: big endian byte order */
dcache_uint32
dcache_uint32_todisk (char *buf, dcache_uint32 num)
{
	unsigned char *p=(unsigned char *)buf;
	p[3]=num&0xff;
	num >>= 8;
	p[2]=num&0x0ff;
	num >>= 8;
	p[1]=num&0x0ff;
	num >>= 8;
	p[0]=num&0x0ff;
	return 4;
}
#define MASK_32BIT (0xffffffff)
dcache_uint32
dcache_pos_todisk (char *buf, dcache_pos num)
{
	dcache_uint32 l1,l2;
	l1=dcache_uint32_todisk(buf,num>>32);
	l2=dcache_uint32_todisk(buf+4,num & MASK_32BIT);
	return l1+l2;
}
/* num < 0 must never be written to disk anyway. No need to check this? */
dcache_uint32
dcache_reclen_todisk (char *buf, dcache_reclen num)
{ return dcache_uint32_todisk(buf,num); }
