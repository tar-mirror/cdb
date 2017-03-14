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

/*                12345678 */
#define BIT_32 (0x80000000)

/* big endian byte order used to trigger problems on the 
 * little endian development system. */
dcache_uint32
dcache_uint32_fromdisk (const char *buf,dcache_uint32 *x)
{
	const unsigned char *p=(const unsigned char *)buf;
	*x=p[3] 
	     + p[2]*0x100UL
		 + p[1]*0x10000UL
		 + p[0]*0x1000000UL;
	return 4;
}

/* get a 64bit signed value from two 32 bit unsigned values. */
dcache_uint32
dcache_pos_fromdisk (const char *buf,dcache_pos *x)
{
	dcache_uint32 u1,u2;
	dcache_uint32 l1,l2;
	l1=dcache_uint32_fromdisk(buf,&u1);
	l2=dcache_uint32_fromdisk(buf+sizeof(u1),&u2);
	if (u1 & BIT_32) /* highest bit set? */	
		return 0;
	*x=(((dcache_pos)u1)<<32)+u2;
	if (*x<0) 
		return 0;
	return l1+l2;
}

dcache_uint32
dcache_reclen_fromdisk (const char *buf,dcache_reclen *x)
{
	dcache_uint32 y;
	dcache_uint32 l;
	l=dcache_uint32_fromdisk(buf,&y);
	if (y & BIT_32)
		return 0;
	*x=y;
	if (*x<0)
		return 0;
	return l;
}

