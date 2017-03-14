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

dcache_reclen 
dcache_read(int rfd,void *vbuf,dcache_reclen len)
{
	char *buf=vbuf;
	dcache_reclen got=0;
	while (len!=got) {
		int r;
		r=read(rfd,buf+got,len-got);
		if (-1==r && errno==EINTR)
			continue;
		if (-1==r) return -1;
		if (0==r) {errno=EIO; return -1; }
		got+=r;
	}
	return got;
}
