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
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h> /* abort - used in sanity checks */
#include "dcache.h"
#include "dcachei.h"

static int 
get_hash(int fd, dcache_reclen keylen, dcache_uint32 *hash)
{
	char buf[512];
	dcache_hash_init(hash);
	while (keylen) {
		int got;
		dcache_reclen r=sizeof(buf);
		if (r>keylen)
			r=keylen;
		got=dcache_read(fd,buf,r);
		if (-1==got)
			return -1;
		dcache_hash_update(hash,buf,got);
		keylen-=got;
	}
	dcache_hash_finish(hash);
	return 0;
}

static int
pos_hash_to_slot(dcache *c, dcache_pos o, dcache_uint32 h, 
	dcache_uint32 *slot)
{
	dcache_uint32 hslot;
	dcache_uint32 loop=0;
	/* note: we need to change c->loop here because dcache_delete
	 * needs it. */
	while (1) {
		if (loop==c->elements)	
			return 0; /* can't happen */
		hslot=(h+loop++)%c->elements;
		if (!c->ep[hslot])
			return 0;
		if (c->hp[hslot]==h && c->ep[hslot]==o) {
			*slot=hslot;
			c->hash=h;
			c->loop=loop;
			return 1;
		}
	}
}

static int
set_free(dcache *c)
{
	dcache_reclen keylen;
	dcache_reclen datalen;
	dcache_pos o;
	char buf[DC_HEADLEN];
	dcache_reclen got;
	dcache_uint32 hash;
	dcache_uint32 slot;

	o=c->o;
	if (o==c->maxsize)
		o=0;

	if (-1==dcache_seek(c->fd,o+c->dataoffset))
		return -1;
	got=dcache_read(c->fd,buf,sizeof(buf));
	if (-1==got)
		return -1;

	if (0==dcache_reclen_fromdisk(buf+DC_OFF_KEYLEN,&keylen))
		goto invalid;
	if (0==dcache_reclen_fromdisk(buf+DC_OFF_DATALEN,&datalen))
		goto invalid;

	if (keylen!=0) { /* if not already deleted */
		/* try to find the slot of the record */
		if (-1==get_hash(c->fd,keylen, &hash))
			return -1;
		if (0==pos_hash_to_slot(c,o+c->dataoffset,hash,&slot)) {
			write(2,"\ndcache: warning: failed to find record\n",40);
			goto invalid;
		}
		if (c->dcallback) {
			dcache_uint32 ttl1,ttl2;
			dcache_time expire;
			int ret;
			/* XXX knowns that dcache_time is 64bit */
			if (0==dcache_uint32_fromdisk(buf+DC_OFF_EXPIRE,&ttl1))
				goto invalid;
			if (0==dcache_uint32_fromdisk(buf+DC_OFF_EXPIRE2,&ttl2))
				goto invalid;
			expire   = ttl1;
			expire <<= 32;
			expire  += ttl2;
			c->keypos=o+c->dataoffset+DC_HEADLEN;
			c->keylen=keylen;
			c->datapos=c->keypos+c->keylen;
			c->datalen=datalen;
			c->datatime=expire;
			ret=c->dcallback(c);
			if (ret!=0)
				return -1;
		}
		dcache_delete(c);
	}
	o+=DC_HEADLEN+keylen+datalen;
	if (o>=c->l)
		o=c->maxsize;
	c->o=o;
	if (c->flag_autosync)
		return dcache_sync(c);
	else
		return dcache_store_vars(c); /* for dcachestats */
  invalid:
	errno=EINVAL;
	return -1;
}

static int
freeloop(dcache *c, dcache_pos need)
{
	while (1) {
		dcache_pos have;
		/* how much free space? */
		if (c->n > c->o)      have=c->maxsize - c->n;
		else                  have=c->o - c->n;

		if (need<=have)
			break;
		if (c->o==c->maxsize) {
			c->o=0;
			c->l=c->n;
			c->n=0;
			continue;
		}
		if (c->n > c->o) {
			c->l=c->n;
			c->n=0;
		} else if (-1==set_free(c))
			return -1;
	}
	while (c->slotuselimit<=c->inuse)
		if (-1==set_free(c)) 	
			return -1;
	return 0;
}
static int
check_size(dcache *c, dcache_reclen keylen, dcache_reclen datalen, 
	dcache_reclen *need)
{
	dcache_pos lencheck;
	/* dcache_delete adds keylen to datalen. So make sure the sum
	 * fits into 31 bits */
	if (keylen==0) { /* flag for "deleted" */
		errno=EINVAL;
		return -1;
	}
	lencheck=keylen;
	lencheck+=datalen;
	lencheck+=DC_HEADLEN; /* for ease of programming, not strictly necessary */
	if (lencheck > DCACHE_RECLEN_MAX) {
		errno=EINVAL;
		return -1;
	}
	if (lencheck > c->maxsize) {
		errno=ENOMEM;
		return -1;
	}
	*need=lencheck;
	return 0;
}

int
dcache_enter(dcache *c, const void *key, dcache_reclen keylen,
	dcache_data *data, dcache_time timevalue)
{
	dcache_uint32 hash;
	dcache_pos hslot;
	dcache_uint32 ui;
	dcache_reclen need;
	dcache_reclen datalen;
	dcache_data *datap;
	dcache_data vec[2];
	dcache_uint32 ttl1;
	dcache_uint32 ttl2;
	char recbuf[DC_HEADLEN];

	dcache_hash(&hash,key,keylen);
	hslot=hash%c->elements;
	if (c->o < 0) abort();
	if (c->n < 0) abort();
	if (c->o > c->maxsize) abort();
	if (c->n > c->maxsize) abort();

	for (datap=data, datalen=0; datap; datap=datap->next)
		datalen+=datap->len;

	if (-1==check_size(c,keylen,datalen,&need))
		return -1;

	if (c->t_buf)
		return dcache_trans_enter(c,key,keylen,data,timevalue);

	/* make sure there is enough space and slots */
	if (-1==freeloop(c,need))
		return -1;

	/* create header */
	dcache_reclen_todisk(recbuf+DC_OFF_KEYLEN,keylen);
	dcache_reclen_todisk(recbuf+DC_OFF_DATALEN,datalen);
	ttl1=timevalue>>32; /* knowns that dcache_time is 64bit */
	ttl2=timevalue&0xffffffff;
	dcache_uint32_todisk(recbuf+DC_OFF_EXPIRE,ttl1);
	dcache_uint32_todisk(recbuf+DC_OFF_EXPIRE2,ttl2);

	/* find a free slot in the tables */
	for (ui=0;ui<c->elements;ui++) {
		dcache_uint32 uj; /* keep gcc quiet */
		uj=(ui+hslot)%c->elements;
		if (c->ep[uj]==0) {
			hslot=uj;
			break;
		}
	}
	/* we _know_ that there is a free slot. See above. */
	/* therefore hslot now is OK */

	/* write the record */
	vec[0].p=recbuf;
	vec[0].len=DC_HEADLEN;
	vec[0].next=&vec[1];
	vec[1].p=key;
	vec[1].len=keylen;
	vec[1].next=data;
	if (-1==dcache_seek(c->fd,c->n+c->dataoffset))
		return -1;
	if (-1==dcache_writev(c->fd,vec))
		return -1;

	c->ep[hslot]=c->n+c->dataoffset;
	c->hp[hslot]=hash;
	c->inuse++;
	c->inserted++;
	c->n+=need;
	if (c->n > c->l)
		c->l=c->n;
	if (c->flag_autosync) {
		if (-1==dcache_sync(c))
			return -1; /* database corrupted? */
	} else {
		/* for dcachestats */
		if (-1==dcache_store_vars(c))
			return -1;
	}
	return 0;
}
