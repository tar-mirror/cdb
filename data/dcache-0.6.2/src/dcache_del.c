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
#include <stdlib.h>
#include "dcache.h"
#include "dcachei.h"

#define WANTSLOT(hash) ((hash) % c->elements)
int 
dcache_delete(dcache *c)
{
	dcache_pos slot;
	dcache_uint32 i;
	dcache_pos fill;
	dcache_reclen kl,dl;
	char head[DC_HEADLEN];

	slot=(c->hash+c->loop-1) % c->elements;

	if (!c->ep[slot])
		return 0; /* already gone. Likely to be users error */
	if (-1==dcache_seek(c->fd,c->ep[slot]))
		return -1;
	if (-1==dcache_read(c->fd,head,sizeof(head)))
		return -1;
	/* "deleted" means key is 0 bytes long */
	if (0==dcache_reclen_fromdisk(head+DC_OFF_KEYLEN,&kl)) {
		errno=EINVAL;
		return -1;
	}
	if (0==dcache_reclen_fromdisk(head+DC_OFF_DATALEN,&dl)) {
		errno=EINVAL;
		return -1;
	}
	if (c->t_buf) {
		/* part of a transaction */
		char *k;
		int x;
		k=malloc(kl);
		if (!k) {
			errno=ENOMEM;
			return -1;
		}
		if (-1==dcache_read(c->fd,k,kl)) {
			x=errno;
			free(k);
			errno=x;
			return -1;
		}
		if (-1==dcache_trans_delete(c,k,kl,c->ep[slot])) {
			x=errno;
			free(k);
			errno=x;
			return -1;
		}
		free(k);
		return 0;
	}
	dl+=kl;
	dcache_reclen_todisk(head+DC_OFF_KEYLEN,0);
	dcache_reclen_todisk(head+DC_OFF_DATALEN,dl);

	if (-1==dcache_seek(c->fd,c->ep[slot]))
		return -1;
	if (-1==dcache_write(c->fd,head,sizeof(head)))
		return -1;

	/* now fix the collision chain. Hacker be careful - this is a little
	 * bit difficult. To state it simply: move all elements of a collsion
	 * chain into another place without breaking a collision chain.
	 * This is made simpler by the fact that there _are_ hash slots
	 * free - the 75% margin ensures that.
	 */
	fill=slot;
	for (i=1;i<c->elements;i++) {
		dcache_pos here;
		dcache_pos belongs;
		here=(slot+i) % c->elements;
		if (!c->ep[here])
			break;
		belongs=WANTSLOT(c->hp[here]);
		if (here==belongs) 
			continue;
		if (here > belongs) {
			if (fill < belongs)
				continue;
			if (fill > here)
				continue;
		} else {
			if (fill > here && fill <belongs)
				continue;
		}
		c->ep[fill]=c->ep[here];
		c->hp[fill]=c->hp[here];
		c->moves++; /* statistics */
		fill=here;
	}

	c->ep[fill]=0;
	c->inuse--;
	c->deleted++;
	return 1;
}
