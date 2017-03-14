/*
 * Copyright (C) 2001-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/LGPL-2.1
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include <sys/types.h>
#include <sys/mman.h>
#include "dcache.h"


void
dcache_free(dcache *c) 
{
	munmap((void *)c->mapped,c->mapsize);
	c->mapped=0;
}
