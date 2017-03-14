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
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "dcache.h"
#include "dcachei.h"

/* the "usual" code to deal with incomplete writev. Should be a standard
 * library function. */
static int
do_writev(int fd, struct iovec *vec, unsigned int count)
{
	while (1) {
		int r;
		r=writev(fd,vec,count);
		if (-1==r) {
			if (errno==EINTR || errno==EWOULDBLOCK || errno==EAGAIN)
				continue;
			return -1;
		}
		while (r) {
			if (r>=(int) vec[0].iov_len) {
				r-=vec[0].iov_len;
				vec++;
				count--;
				if (!count)
					break;
			} else {
				vec[0].iov_len-=r;
				vec[0].iov_base=r+ ((char *)vec[0].iov_base);
				r=0;
			}
		}
		if (count==0)
			return 0;
	}
}

#define MAXVEC 10
dcache_reclen
dcache_writev(int fd, dcache_data *d)
{
	struct iovec i[MAXVEC];
	unsigned int j;

	/* convert dcache_data to iovec. stop as soon as input has to 
	 * be read from a file. */
	for (j=0;d && d->p && j<MAXVEC; d=d->next) {
		union { const char *cc; char *c;} u; /* disqualify const */
		if (d->len==0)
			continue;
		u.cc=d->p;
		i[j].iov_base=u.c;
		i[j].iov_len=d->len;
		j++;
	}
	if (j && -1==do_writev(fd,i,j))
		return -1;
	while (d) {
		if (d->p) {
			if (-1==dcache_write(fd,d->p,d->len))
				return -1;
		} else {
			char buf[8192];
			dcache_reclen l=d->len;
			while (l) {
				dcache_reclen r;
				r=dcache_read(d->fd,buf,
					l > (dcache_reclen) sizeof(buf)
						? (dcache_reclen) sizeof(buf) : l);
				if (-1==r)
					return -1;
				if (-1==dcache_write(fd,buf,r))
					return -1;
				l-=r;
			}
		}
		d=d->next;
	}
	return 0;
}

