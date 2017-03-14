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
#include <errno.h>
#include "dcache.h"
#include "dcachei.h"

void dcache_lookupstart(dcache *c) { c->loop=0; }

/* compare the next 'l' bytes read from 'fd' with the 'l' bytes starting
 * at 'key'. */
static int keyequal(int fd, const char *key, dcache_reclen l)
{
	char buf[512]; /* 2, 3 and 5000 are ok also */
	while (l) {
		dcache_reclen r;
		dcache_reclen i;
		i=sizeof(buf);
		if (i>l)
			i=l;
		r=dcache_read(fd,buf, i);
		if (-1==r)
			return -1;
		for (i=0;i<r;i++)
			if (key[i]!=buf[i])
				return 0;
		key+=r;
		l-=r;
	}
	return 1;
}
static int
ifind(dcache *c, dcache_reclen keylen, const char *key)
{
	dcache_pos should_slot;
	dcache_pos slot;
	dcache_reclen e_keylen;
	char head[DC_HEADLEN];
	if (keylen==0) /* fast exit: deleted record cannot be found */
		return 0;

	if (!c->loop) 
		dcache_hash(&c->hash,key,keylen);
	should_slot=(c->hash)%c->elements;

	/* search for empty slot or slot with hash 'hash' */
	do {
		slot=(should_slot+c->loop++)%c->elements;
		if (c->ep[slot]==0)
			return 0; /* not found */
		if (c->hp[slot]==c->hash) {
			/* may be */
			break;
		}
      trynext:
	  	/* note that this never should happen: 25% of the tables are
		 * left free for performance reasons */
	    if (c->loop==c->elements)
			return 0;
	} while (1);

	if (-1==dcache_seek(c->fd,c->ep[slot]))
		return -1;
	if (-1==dcache_read(c->fd,head,sizeof(head)))
		return -1;

	if (0==dcache_reclen_fromdisk(head+DC_OFF_KEYLEN,&e_keylen))
		goto invalid;
	if (e_keylen!=keylen) /* catches also "deleted" with keylen == 0 */
		goto trynext;

	switch(keyequal(c->fd,key,keylen)) {
	case 0: goto trynext;
	case -1: return -1;
	}
	c->datapos=c->ep[slot]+DC_HEADLEN+keylen;

	/* found a record matching key */

	if (0==dcache_reclen_fromdisk(head+DC_OFF_DATALEN,&c->datalen)) 
		goto invalid;
	{
		/* XXX knows that dcache_time is 64 bits */
		dcache_uint32 x;
		dcache_uint32 y;
		if (0==dcache_uint32_fromdisk(head+DC_OFF_EXPIRE,&x))
			goto invalid;
		if (0==dcache_uint32_fromdisk(head+DC_OFF_EXPIRE2,&y))
			goto invalid;
		c->datatime=(((dcache_time)x)<<32) +y;
	}
	return 1;
  invalid:
	errno=EINVAL;
	return -1;
}

int 
dcache_lookup(dcache *lv0,const char *key, dcache_reclen keylen)
{
	dcache_lookupstart(lv0);
	return dcache_lookupnext(lv0,key,keylen);
}

int dcache_lookupnext(dcache *lv0,const char *key, dcache_reclen keylen)
{
	switch(ifind(lv0,keylen,key)) {
	case -1: return -1;
	case 0: return 0;
	}
	return 1;
}


