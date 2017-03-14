/*
 * Copyright (C) 2001-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/LGPL-2.1
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include <errno.h>
#include "dcache.h"
#include "dcachei.h"

void 
dcache_walkstart(dcache *c)
{	
	c->walkpos=c->o;
	c->walking=0; /* 1 means "break if at c->n" */
	if (c->n==0)
		c->walking=1; /* the thing is empty */
}

int dcache_walk(dcache *c)
{
	char head[DC_HEADLEN];
	while (1) {
		dcache_reclen kl,dl;
		if (c->walkpos == c->maxsize && !c->n)
			return 0;
		if (c->walkpos == c->n && c->walking)
			return 0; /* last */
		if (c->walkpos == c->l)
			c->walkpos=0;
		if (c->walkpos == c->maxsize) /* redundant check */
			c->walkpos=0;
		c->walking=1;
		if (-1==dcache_seek(c->fd,c->walkpos+c->dataoffset))
			return -1;
		if (-1==dcache_read(c->fd,head,sizeof(head)))
			return -1;
		if (0==dcache_reclen_fromdisk(head+DC_OFF_KEYLEN,&kl))
			goto invalid;
		if (0==dcache_reclen_fromdisk(head+DC_OFF_DATALEN,&dl))
			goto invalid;

		c->walkpos+=DC_HEADLEN+kl+dl;

		if (kl) { /* if not deleted */
			c->keylen=kl;
			c->keypos=c->walkpos+DC_HEADLEN;
			c->datalen=dl;
			c->datapos=c->keypos+kl;
			{
				dcache_uint32 x,y;
				if (0==dcache_uint32_fromdisk(head+DC_OFF_EXPIRE,&x))
					goto invalid;
				if (0==dcache_uint32_fromdisk(head+DC_OFF_EXPIRE2,&y))
					goto invalid;
				c->datatime=(((dcache_time)x)<<32)+y;
			}
			return 1;
		}
	}
	/* not reached */
  invalid:
	errno=EINVAL;
	return -1;
}
