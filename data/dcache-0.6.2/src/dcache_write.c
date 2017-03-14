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

dcache_reclen
dcache_write(int fd, const void *vbuf, dcache_reclen l)
{
	const char *buf=vbuf;
	while (l) {
		dcache_reclen written;
		written=write(fd,buf,l);
		if (-1==written) {
			if (EINTR==errno) continue;
			return -1;
		}
		l-=written;
		buf+=written;
	}
	return (buf-(const char *)vbuf);
}

