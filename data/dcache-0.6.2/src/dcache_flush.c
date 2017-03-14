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
#include <sys/mman.h>
#include <sys/stat.h>
#include "dcache.h"

int 
dcache_sync(dcache *c)
{
	if (-1==dcache_flush(c))
		return -1;
	if (-1==fsync(c->fd))
		return -1;
	return 0;
}

int
dcache_flush(dcache *c)
{
	if (-1==dcache_store_vars(c))
		return -1;
	if (-1==msync(c->mapped,c->mapsize,MS_SYNC|MS_INVALIDATE))
		return -1;
	return 0;
}
